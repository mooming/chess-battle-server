#pragma once

#include <string>
#include <vector>

class ClientStub
{
public:
    ClientStub() : fd_(-1) {}
    ~ClientStub() { close(); }

    void connect(const std::string& host, uint16_t port);
    void close();
    int getFd() const;

    // Protocol methods.
    void sendGreeting(const std::string& name);
    bool receiveGreeting(std::string& serverName, std::string& serverId);
    bool receiveConnectionSucceeded(std::string& message);
    bool receiveConnectionFailed(std::string& reason);

    void inquireSessions();
    bool receiveSessionInfo(uint16_t& sessionId, uint8_t& playerCount,
                            std::string& name, uint8_t& state);

    void sendMove(const std::string& from, const std::string& to);
    void sendResign();
    void sendVerifyState(const std::vector<std::string>& pawns);
    void sendRequestGameState();
    bool receiveGameState(std::string& fen);

private:
    int fd_;
};
