#include "SessionManager.h"

#include <algorithm>
#include <sstream>

namespace session
{

SessionManager::SessionManager() = default;

uint16_t SessionManager::createSession(const std::string& name)
{
    Session s;
    s.id = nextSessionId_++;
    s.name = name;
    s.game.reset();
    s.state = 0; // waiting
    sessions_[s.id] = std::move(s);
    return sessions_.rbegin()->first;
}

bool SessionManager::joinSession(ClientContext& client, uint16_t sessionId)
{
    auto it = sessions_.find(sessionId);
    if (it == sessions_.end()) return false;
    Session& sess = it->second;

    if (sess.state != 0) return false; // not waiting
    if (sess.players.size() >= 2) return false; // full

    client.sessionId = sessionId;
    sess.players.push_back(&client);

    // If two players, start the game.
    if (sess.players.size() == 2)
    {
        sess.game.reset();
        sess.state = 1; // active
    }

    return true;
}

void SessionManager::leaveSession(ClientContext& client)
{
    if (client.sessionId == 0) return;
    auto it = sessions_.find(client.sessionId);
    if (it == sessions_.end()) return;

    Session& sess = it->second;
    sess.players.erase(
        std::remove(sess.players.begin(), sess.players.end(), &client),
        sess.players.end());

    if (sess.players.empty())
    {
        endSession(client.sessionId);
    }
    else if (sess.state == 1)
    {
        // One player left — game ends as resignation by the leaver.
        // The remaining player wins.
        ClientContext* winner = sess.players[0];
        sess.game.status = (winner->name == "White") ? engine::GameStatus::WhiteWins
                                                       : engine::GameStatus::BlackWins;
        sess.state = 2; // finished
    }

    client.sessionId = 0;
    client.game = nullptr;
}

void SessionManager::endSession(uint16_t sessionId)
{
    auto it = sessions_.find(sessionId);
    if (it == sessions_.end()) return;
    it->second.state = 2; // finished
    it->second.players.clear();
}

std::vector<SessionManager::SessionInfo> SessionManager::listSessions() const
{
    std::vector<SessionInfo> result;
    for (const auto& [id, sess] : sessions_)
    {
        if (sess.state == 2) continue; // skip finished
        SessionInfo info;
        info.id = id;
        info.playerCount = static_cast<uint8_t>(sess.players.size());
        info.name = sess.name;
        info.state = sess.state;
        result.push_back(std::move(info));
    }
    return result;
}

const SessionManager::SessionInfo* SessionManager::getSession(uint16_t sessionId) const
{
    auto it = sessions_.find(sessionId);
    if (it == sessions_.end()) return nullptr;
    return nullptr; // placeholder — listSessions is the primary query method
}

engine::Game* SessionManager::getGame(uint16_t sessionId)
{
    auto it = sessions_.find(sessionId);
    if (it == sessions_.end()) return nullptr;
    return &it->second.game;
}

void SessionManager::addClient(ClientContext* client)
{
    clientsByFd_[client->socketFd] = client;
    if (!client->id.empty())
        clientsById_[client->id] = client;
}

void SessionManager::removeClient(int socketFd)
{
    auto it = clientsByFd_.find(socketFd);
    if (it == clientsByFd_.end()) return;

    ClientContext* client = it->second;
    leaveSession(*client);

    if (!client->id.empty())
        clientsById_.erase(client->id);
    clientsByFd_.erase(it);
}

ClientContext* SessionManager::findClient(int socketFd)
{
    auto it = clientsByFd_.find(socketFd);
    return (it != clientsByFd_.end()) ? it->second : nullptr;
}

ClientContext* SessionManager::findClientById(const std::string& id)
{
    auto it = clientsById_.find(id);
    return (it != clientsById_.end()) ? it->second : nullptr;
}

bool SessionManager::applyMove(ClientContext& client, const std::string& from, const std::string& to)
{
    if (client.sessionId == 0) return false;
    auto it = sessions_.find(client.sessionId);
    if (it == sessions_.end()) return false;

    Session& sess = it->second;
    if (sess.state != 1) return false; // game not active

    engine::Game* game = &sess.game;
    if (!game->makeMove(from, to)) return false;

    // Update client's reference to the game.
    client.game = game;

    // Check for game end.
    if (game->isGameOver())
    {
        sess.state = 2; // finished
    }

    return true;
}

void SessionManager::handleResign(ClientContext& client)
{
    if (client.sessionId == 0) return;
    auto it = sessions_.find(client.sessionId);
    if (it == sessions_.end()) return;

    Session& sess = it->second;
    if (sess.state != 1) return;

    // The resigning player loses. The other player wins.
    engine::Game* game = &sess.game;
    // Determine if the resigning player is White or Black.
    // We can check whose turn it is — if it's the resigning player's turn, they lose.
    // Actually, simpler: the resigning player's color is determined by their position.
    // For now, let's say: if the game's lastMoveColor matches the client, they were White.
    // But we don't track client color explicitly. Let's use a simpler approach:
    // The first player to join is White, the second is Black.
    if (!sess.players.empty() && sess.players[0] == &client)
    {
        game->status = engine::GameStatus::BlackWins;
    }
    else if (sess.players.size() > 1 && sess.players[1] == &client)
    {
        game->status = engine::GameStatus::WhiteWins;
    }
    else
    {
        // Fallback: black wins.
        game->status = engine::GameStatus::BlackWins;
    }

    sess.state = 2; // finished
}

bool SessionManager::verifyGameState(ClientContext& client, std::vector<std::string> pawns)
{
    if (client.sessionId == 0) return false;
    auto it = sessions_.find(client.sessionId);
    if (it == sessions_.end()) return false;

    Session& sess = it->second;
    engine::Game* game = &sess.game;

    // Get authoritative pawn state.
    std::vector<std::string> authoritative;
    game->getPawnState(authoritative);

    // Compare. Sort both for order-independent comparison.
    std::sort(pawns.begin(), pawns.end());
    std::sort(authoritative.begin(), authoritative.end());

    return pawns == authoritative;
}

} // namespace session
