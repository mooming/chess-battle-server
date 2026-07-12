#include "ClientStub.h"

#include "Network/BinaryIO.h"
#include "Protocol/Protocol.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <unistd.h>

void ClientStub::connect(const std::string& host, uint16_t port)
{
    struct addrinfo hints{}, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0)
    {
        std::cerr << "getaddrinfo failed: " << gai_strerror(err) << std::endl;
        return;
    }

    fd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd_ == -1)
    {
        perror("socket");
        freeaddrinfo(res);
        return;
    }

    if (::connect(fd_, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("connect");
        ::close(fd_);
        fd_ = -1;
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    std::cout << "Connected to " << host << ":" << port << std::endl;
}

void ClientStub::sendGreeting(const std::string& name)
{
    using namespace network;
    WritePid(fd_, protocol::kPidGreeting);
    WriteUint16(fd_, 1); // version
    WriteString(fd_, name);
    WriteString(fd_, ""); // no ID on first connect
}

bool ClientStub::receiveGreeting(std::string& serverName, std::string& serverId)
{
    using namespace network;
    uint16_t pid = 0;
    if (!ReadPid(fd_, pid)) return false;
    if (pid != protocol::kPidGreeting) return false;

    uint16_t version = 0;
    if (!ReadUint16(fd_, version)) return false;
    if (!ReadString(fd_, serverName, 63)) return false;
    if (!ReadString(fd_, serverId, 127)) return false;
    return true;
}

bool ClientStub::receiveConnectionSucceeded(std::string& message)
{
    using namespace network;
    uint16_t pid = 0;
    if (!ReadPid(fd_, pid)) return false;
    if (pid != protocol::kPidConnectionSucceeded) return false;
    return ReadString(fd_, message, 255);
}

bool ClientStub::receiveConnectionFailed(std::string& reason)
{
    using namespace network;
    uint16_t pid = 0;
    if (!ReadPid(fd_, pid)) return false;
    if (pid != protocol::kPidConnectionFailed) return false;
    return ReadString(fd_, reason, 255);
}

void ClientStub::inquireSessions()
{
    using namespace network;
    WritePid(fd_, protocol::kPidInquireGameSessions);
}

bool ClientStub::receiveSessionInfo(uint16_t& sessionId, uint8_t& playerCount,
                                     std::string& name, uint8_t& state)
{
    using namespace network;
    uint16_t pid = 0;
    if (!ReadPid(fd_, pid)) return false;
    if (pid != protocol::kPidGameSessionInfo) return false;

    if (!ReadUint16(fd_, sessionId)) return false;
    if (!ReadUint8(fd_, playerCount)) return false;
    uint8_t nameLen = 0;
    if (!ReadUint8(fd_, nameLen)) return false;
    char nameBuf[64] = {};
    if (!ReadFixedArray(fd_, nameBuf, nameLen)) return false;
    name.assign(nameBuf, nameLen);
    if (!ReadUint8(fd_, state)) return false;
    return true;
}

void ClientStub::sendMove(const std::string& from, const std::string& to)
{
    using namespace network;
    WritePid(fd_, protocol::kPidMove);
    char fromBuf[4], toBuf[4];
    std::memcpy(fromBuf, from.c_str(), 4);
    std::memcpy(toBuf, to.c_str(), 4);
    WriteFixedArray(fd_, fromBuf, 4);
    WriteFixedArray(fd_, toBuf, 4);
}

void ClientStub::sendResign()
{
    using namespace network;
    WritePid(fd_, protocol::kPidResign);
}

void ClientStub::sendVerifyState(const std::vector<std::string>& pawns)
{
    using namespace network;
    WritePid(fd_, protocol::kPidVerifyGameState);
    WriteUint8(fd_, static_cast<uint8_t>(pawns.size()));
    for (const auto& p : pawns)
    {
        char buf[4];
        std::memcpy(buf, p.c_str(), 4);
        WriteFixedArray(fd_, buf, 4);
    }
}

void ClientStub::sendRequestGameState()
{
    using namespace network;
    WritePid(fd_, protocol::kPidRequestGameState);
}

bool ClientStub::receiveGameState(std::string& fen)
{
    using namespace network;
    uint16_t pid = 0;
    if (!ReadPid(fd_, pid)) return false;
    if (pid != protocol::kPidGameState) return false;
    return ReadString(fd_, fen, 511);
}

void ClientStub::close()
{
    if (fd_ != -1)
    {
        ::close(fd_);
        fd_ = -1;
    }
}

int ClientStub::getFd() const { return fd_; }
