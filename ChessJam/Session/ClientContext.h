#pragma once

#include "Protocol/Protocol.h"
#include "Engine/Game.h"

#include <string>
#include <memory>

namespace session
{

// Per-client state tracked by the server.
struct ClientContext
{
    int                       socketFd = -1;
    std::string               name;
    std::string               id;         // Server-assigned unique ID
    uint16_t                  version    = 0;
    uint16_t                  sessionId  = 0; // 0 = not in a session
    engine::Game*             game       = nullptr; // nullptr if not in a game
    bool                      authenticated = false;
};

} // namespace session
