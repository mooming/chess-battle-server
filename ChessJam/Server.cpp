#include "Server.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

Server::Server() : listen_fd(-1) {}
Server::~Server() {
    if (listen_fd != -1) close(listen_fd);
}

void Server::start(const std::string& address) {
    // Parse address for optional port
    std::string host = address;
    uint16_t port = 12345; // default port
    auto colonPos = address.find(':');
    if (colonPos != std::string::npos) {
        host = address.substr(0, colonPos);
        port = static_cast<uint16_t>(std::stoi(address.substr(colonPos + 1)));
    }

    // Resolve host
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int err = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
        return;
    }

    listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listen_fd == -1) {
        perror("socket");
        freeaddrinfo(res);
        return;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(listen_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        close(listen_fd);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        return;
    }

    std::cout << "Server listening on " << host << ":" << port << std::endl;

    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        std::cout << "Client connected" << std::endl;
        // Simple handling in same thread for demo
        sendGreeting(client_fd);
        sendConnectionSucceeded(client_fd);
        handleClient(client_fd);
        close(client_fd);
        std::cout << "Client disconnected" << std::endl;
    }
}

void Server::handleClient(int client_fd) {
    // Simple loop: read PID and ignore
    while (true) {
        uint16_t pid = readUint16(client_fd);
        if (pid == 0) break; // client closed
        std::cout << "Received PID: 0x" << std::hex << pid << std::dec << std::endl;
        // For demo, just echo back the PID
        // In real implementation, process per protocol
    }
}

void Server::sendGreeting(int fd) {
    uint16_t pid = 0x0001; // Greeting
    uint16_t version = 1;
    const std::string name = "ChessServer";
    uint16_t nameLen = static_cast<uint16_t>(name.size());
    writeUint16(fd, pid);
    writeUint16(fd, version);
    writeUint16(fd, nameLen);
    write(fd, name.c_str(), nameLen);
}

void Server::sendConnectionSucceeded(int fd) {
    uint16_t pid = 0x0002; // ConnectionSucceeded
    const std::string msg = "Welcome";
    uint16_t msgLen = static_cast<uint16_t>(msg.size());
    writeUint16(fd, pid);
    writeUint16(fd, msgLen);
    write(fd, msg.c_str(), msgLen);
}

void Server::sendConnectionFailed(int fd, const std::string& reason) {
    uint16_t pid = 0x0003; // ConnectionFailed
    uint16_t len = static_cast<uint16_t>(reason.size());
    writeUint16(fd, pid);
    writeUint16(fd, len);
    write(fd, reason.c_str(), len);
}

uint16_t Server::readUint16(int fd) {
    uint16_t val;
    ssize_t n = read(fd, &val, sizeof(val));
    if (n != sizeof(val)) return 0;
    return ntohs(val);
}

void Server::writeUint16(int fd, uint16_t value) {
    uint16_t net = htons(value);
    write(fd, &net, sizeof(net));
}

void Server::writeString(int fd, const std::string& str) {
    write(fd, str.c_str(), str.size());
}
