#pragma once

#include <cstdint>

namespace protocol
{

// ── PID Constants ──────────────────────────────────────────────────────
// Grouped by high byte for future expansion:
//   0x00xx  Handshake
//   0x01xx  Session management
//   0x02xx  Gameplay
//   0x03xx  State verification
//   0x04xx  Broadcast / state sync

// Handshake (0x00xx)
constexpr uint16_t kPidGreeting                = 0x0001;
constexpr uint16_t kPidConnectionSucceeded     = 0x0002;
constexpr uint16_t kPidConnectionFailed        = 0x0003;

// Session management (0x01xx)
constexpr uint16_t kPidInquireGameSessions     = 0x0100;
constexpr uint16_t kPidGameSessionInfo         = 0x0101;

// Gameplay (0x02xx)
constexpr uint16_t kPidMove                    = 0x0200;
constexpr uint16_t kPidResign                  = 0x0201;
constexpr uint16_t kPidPawnStateChange         = 0x0202;

// State verification (0x03xx)
constexpr uint16_t kPidVerifyGameState         = 0x0300;

// Broadcast / state sync (0x04xx)
constexpr uint16_t kPidRequestGameState        = 0x0400;
constexpr uint16_t kPidGameState               = 0x0401;

// ── Message Structures (in-memory representation) ─────────────────────
// These are the C++ representations of each message. They are serialized
// to/from the wire using the BinaryIO helpers in Network/BinaryIO.h.

struct Greeting
{
    uint16_t version;
    char     name[64];
    uint16_t nameLen;
    char     id[128];
    uint16_t idLen;
};

struct ConnectionSucceeded
{
    char     message[256];
    uint16_t messageLen;
};

struct ConnectionFailed
{
    char     reason[256];
    uint16_t reasonLen;
};

struct InquireGameSessions
{
    // No payload — just PID.
};

struct GameSessionInfo
{
    uint16_t sessionId;
    uint8_t  playerCount;
    char     name[64];
    uint16_t nameLen;
    uint8_t  state; // 0=waiting, 1=active, 2=finished
};

struct Move
{
    char from[4];  // Pawn locator (e.g. "WKD1")
    char to[4];    // Destination pawn locator (e.g. "WPd2")
};

struct Resign
{
    // No payload — just PID.
};

struct PawnStateChange
{
    char locator[4];  // The pawn that changed
    uint8_t newState; // 0=removed, 1=promoted, etc.
    char promoPiece;  // If promoted, the new piece type ('P','R','N','B','Q')
};

struct VerifyGameState
{
    uint8_t  pawnCount;
    char     pawns[256]; // Concatenated PawnLocator strings (4 bytes each)
};

struct RequestGameState
{
    // No payload — just PID.
};

struct GameState
{
    char     fen[512];
    uint16_t fenLen;
};

} // namespace protocol
