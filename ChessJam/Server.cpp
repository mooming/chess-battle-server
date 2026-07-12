#include "Server.h"

#include "Network/BinaryIO.h"
#include "Protocol/Protocol.h"
#include "Protocol/WireFormat.h"
#include "Protocol/Types.h"

#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

// Global flag for graceful shutdown.
static volatile bool gRunning = false;

static void signalHandler(int)
{
    gRunning = false;
}

Server::Server() = default;

Server::~Server()
{
    if (listenFd_ != -1)
    {
        ::close(listenFd_);
        listenFd_ = -1;
    }
}

void Server::start(const std::string& address)
{
    std::string host = address;
    uint16_t port = 12345;
    auto colonPos = address.find(':');
    if (colonPos != std::string::npos)
    {
        host = address.substr(0, colonPos);
        port = static_cast<uint16_t>(std::stoi(address.substr(colonPos + 1)));
    }

    struct addrinfo hints{}, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
        return;
    }

    listenFd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listenFd_ == -1)
    {
        perror("socket");
        freeaddrinfo(res);
        return;
    }

    int opt = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (::bind(listenFd_, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind");
        ::close(listenFd_);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    if (::listen(listenFd_, 5) == -1)
    {
        perror("listen");
        ::close(listenFd_);
        return;
    }

    // Set up signal handler for graceful shutdown.
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    gRunning = true;
    running_ = true;

    std::cout << "Server listening on " << host << ":" << port << std::endl;

    while (gRunning)
    {
        struct sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = ::accept(listenFd_, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd == -1)
        {
            if (gRunning) perror("accept");
            continue;
        }

        std::cout << "Client connected from "
                  << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port)
                  << std::endl;

        std::thread([this, clientFd]() {
            handleClient(clientFd);
        }).detach();
    }

    std::cout << "Server shutting down." << std::endl;
}

// ── Client Handler ─────────────────────────────────────────────────────

void Server::handleClient(int clientFd)
{
    using namespace network;

    session::ClientContext client;
    client.socketFd = clientFd;

    // ── Step 1: Receive client Greeting ──────────────────────────────
    uint16_t pid = 0;
    if (!ReadPid(clientFd, pid)) goto cleanup;
    if (pid != protocol::kPidGreeting)
    {
        sendConnectionFailed(clientFd, "Expected Greeting");
        goto cleanup;
    }

    if (!ReadUint16(clientFd, client.version)) goto cleanup;
    if (!ReadString(clientFd, client.name, protocol::kMaxEntityNameLen)) goto cleanup;
    if (!ReadString(clientFd, client.id, 127)) goto cleanup;

    std::cout << "Client greeting: name=" << client.name
              << " version=" << client.version
              << " id='" << client.id << "'" << std::endl;

    // ── Step 2: Issue unique ID and send Server Greeting ─────────────
    static uint16_t nextId = 1;
    {
        std::ostringstream oss;
        oss << "client_" << nextId++;
        client.id = oss.str();
    }

    sendGreeting(clientFd, 1, "ChessServer", client.id);

    // ── Step 3: ConnectionSucceeded ──────────────────────────────────
    sendConnectionSucceeded(clientFd, "Welcome to Chess Battle Server");
    client.authenticated = true;

    std::cout << "Client authenticated: " << client.id << std::endl;
    sessionMgr_.addClient(&client);

    // ── Step 4: Message loop ─────────────────────────────────────────
    while (gRunning)
    {
        if (!ReadPid(clientFd, pid)) break;

        bool ok = true;
        switch (pid)
        {
            case protocol::kPidInquireGameSessions:
                ok = processSessionRequest(clientFd, client);
                break;
            case protocol::kPidMove:
            {
                char from[4] = {}, to[4] = {};
                if (!ReadFixedArray(clientFd, from, 4)) ok = false;
                if (!ReadFixedArray(clientFd, to, 4)) ok = false;
                if (ok)
                    ok = processMove(clientFd, client, std::string(from, 4), std::string(to, 4));
                break;
            }
            case protocol::kPidResign:
                processResign(clientFd, client);
                break;
            case protocol::kPidVerifyGameState:
                ok = processVerifyState(clientFd, client);
                break;
            case protocol::kPidRequestGameState:
                ok = processRequestGameState(clientFd, client);
                break;
            default:
                sendConnectionFailed(clientFd, "Unknown PID: 0x" + std::to_string(pid));
                ok = false;
                break;
        }

        if (!ok) break;
    }

cleanup:
    std::cout << "Client disconnected: " << client.id << std::endl;
    sessionMgr_.removeClient(clientFd);
    ::close(clientFd);
}

// ── Protocol Handlers ──────────────────────────────────────────────────

void Server::sendGreeting(int fd, uint16_t version, const std::string& name, const std::string& id)
{
    using namespace network;
    WritePid(fd, protocol::kPidGreeting);
    WriteUint16(fd, version);
    WriteString(fd, name);
    WriteString(fd, id);
}

void Server::sendConnectionSucceeded(int fd, const std::string& message)
{
    using namespace network;
    WritePid(fd, protocol::kPidConnectionSucceeded);
    WriteString(fd, message);
}

void Server::sendConnectionFailed(int fd, const std::string& reason)
{
    using namespace network;
    WritePid(fd, protocol::kPidConnectionFailed);
    WriteString(fd, reason);
}

bool Server::processSessionRequest(int fd, session::ClientContext& client)
{
    using namespace network;
    auto sessions = sessionMgr_.listSessions();
    for (const auto& info : sessions)
    {
        WritePid(fd, protocol::kPidGameSessionInfo);
        WriteUint16(fd, info.id);
        WriteUint8(fd, info.playerCount);
        WriteUint8(fd, static_cast<uint8_t>(info.name.size()));
        WriteBytes(fd, info.name.data(), info.name.size());
        WriteUint8(fd, info.state);
    }
    return true;
}

bool Server::processMove(int fd, session::ClientContext& client,
                         const std::string& from, const std::string& to)
{
    using namespace network;

    if (!sessionMgr_.applyMove(client, from, to))
    {
        sendConnectionFailed(fd, "Invalid move");
        return false;
    }

    std::cout << "Move: " << from << " -> " << to << " by " << client.id << std::endl;

    // If game ended, send final state.
    if (client.game && client.game->isGameOver())
    {
        std::string fen = client.game->getFen();
        WritePid(fd, protocol::kPidGameState);
        WriteUint16(fd, static_cast<uint16_t>(fen.size()));
        WriteBytes(fd, fen.data(), fen.size());
    }

    return true;
}

void Server::processResign(int fd, session::ClientContext& client)
{
    std::cout << "Resign by " << client.id << std::endl;
    sessionMgr_.handleResign(client);
    sendConnectionSucceeded(fd, "Game ended by resignation.");
}

bool Server::processVerifyState(int fd, session::ClientContext& client)
{
    using namespace network;

    uint8_t pawnCount = 0;
    if (!ReadUint8(fd, pawnCount)) return false;

    std::vector<std::string> pawns;
    for (uint8_t i = 0; i < pawnCount; ++i)
    {
        char loc[4];
        if (!ReadFixedArray(fd, loc, 4)) return false;
        pawns.emplace_back(loc, 4);
    }

    if (!sessionMgr_.verifyGameState(client, pawns))
    {
        std::cerr << "Client " << client.id << " failed VerifyGameState!" << std::endl;
        sendConnectionFailed(fd, "Game state mismatch — banned");
        return false;
    }

    return true;
}

bool Server::processRequestGameState(int fd, session::ClientContext& client)
{
    using namespace network;

    if (client.sessionId == 0 || !client.game)
    {
        sendConnectionFailed(fd, "Not in a game session");
        return false;
    }

    std::string fen = client.game->getFen();
    WritePid(fd, protocol::kPidGameState);
    WriteUint16(fd, static_cast<uint16_t>(fen.size()));
    WriteBytes(fd, fen.data(), fen.size());
    return true;
}

void Server::closeClient(int fd)
{
    ::close(fd);
}
