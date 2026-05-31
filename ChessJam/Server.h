#pragma once

#include <string>

class Server {
public:
    Server();
    ~Server();
    // Starts server at given address (host) and optional port (default 12345)
    void start(const std::string& address);
private:
    int listen_fd;
    void handleClient(int client_fd);
    void sendGreeting(int fd);
    void sendConnectionSucceeded(int fd);
    void sendConnectionFailed(int fd, const std::string& reason);
    uint16_t readUint16(int fd);
    void writeUint16(int fd, uint16_t value);
    void writeString(int fd, const std::string& str);
};