#pragma once

#include "Session/SessionManager.h"

#include <string>

class Server
{
public:
    Server();
    ~Server();

    // Start the server on the given address (host:port). Default port 12345.
    void start(const std::string& address);

private:
    int listenFd_ = -1;
    session::SessionManager sessionMgr_;
    bool running_ = false;

    void handleClient(int clientFd);
    void sendGreeting(int fd, uint16_t version, const std::string& name, const std::string& id);
    void sendConnectionSucceeded(int fd, const std::string& message);
    void sendConnectionFailed(int fd, const std::string& reason);
    bool processSessionRequest(int fd, session::ClientContext& client);
    bool processMove(int fd, session::ClientContext& client, const std::string& from, const std::string& to);
    void processResign(int fd, session::ClientContext& client);
    bool processVerifyState(int fd, session::ClientContext& client);
    bool processRequestGameState(int fd, session::ClientContext& client);

    // Helper: close the client socket.
    void closeClient(int fd);
};
