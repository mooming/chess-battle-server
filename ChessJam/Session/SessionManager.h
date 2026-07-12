#pragma once

#include "ClientContext.h"
#include "Engine/Game.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace session
{

// Manages all game sessions.
class SessionManager
{
public:
    SessionManager();
    ~SessionManager() = default;

    // ── Session lifecycle ──────────────────────────────────────────────

    // Create a new session. Returns session ID, or 0 on failure.
    uint16_t createSession(const std::string& name);

    // Join a session by client ID. Returns true if successful.
    bool joinSession(ClientContext& client, uint16_t sessionId);

    // Leave a session (called when client disconnects).
    void leaveSession(ClientContext& client);

    // End a session by ID.
    void endSession(uint16_t sessionId);

    // ── Queries ────────────────────────────────────────────────────────

    // Get all sessions (for InquireGameSessions).
    struct SessionInfo
    {
        uint16_t id;
        uint8_t  playerCount;
        std::string name;
        uint8_t  state; // 0=waiting, 1=active, 2=finished
    };
    std::vector<SessionInfo> listSessions() const;

    // Get session by ID. Returns nullptr if not found.
    const SessionInfo* getSession(uint16_t sessionId) const;

    // Get the Game for a session.
    engine::Game* getGame(uint16_t sessionId);

    // ── Client tracking ────────────────────────────────────────────────

    void addClient(ClientContext* client);
    void removeClient(int socketFd);
    ClientContext* findClient(int socketFd);
    ClientContext* findClientById(const std::string& id);

    // ── Move enforcement ───────────────────────────────────────────────

    // Apply a move from a client. Validates and broadcasts.
    // Returns true if the move was valid and applied.
    bool applyMove(ClientContext& client, const std::string& from, const std::string& to);

    // Handle a resignation from a client.
    void handleResign(ClientContext& client);

    // Verify a client's game state. Returns true if valid.
    bool verifyGameState(ClientContext& client, std::vector<std::string> pawns);

private:
    struct Session
    {
        uint16_t                     id;
        std::string                  name;
        engine::Game                 game;
        std::vector<ClientContext*>  players;
        uint8_t                      state = 0; // waiting
    };

    std::map<uint16_t, Session> sessions_;
    std::map<int, ClientContext*> clientsByFd_;
    std::map<std::string, ClientContext*> clientsById_;
    uint16_t nextSessionId_ = 1;
    uint16_t nextClientId_ = 1;
};

} // namespace session
