#pragma once

// Wire format byte-level definitions for all protocol messages.
// These are NOT C++ structs — they are byte-layout references.
// All multi-byte integers are big-endian (network byte order).
//
// Usage: These constants define the total wire size of each message.
//        Actual serialization is done via BinaryIO functions.

namespace protocol::wire
{

// ── Handshake (0x00xx) ───────────────────────────────────────────────

// Greeting: [PID:2][Version:2][NameLen:2][Name:N][IDLen:2][ID:M]
constexpr uint16_t kWireGreetingFixedSize = 6; // PID + Version + NameLen
constexpr uint16_t kWireGreetingMaxSize   = 6 + 64 + 2 + 128;

// ConnectionSucceeded: [PID:2][MsgLen:2][Message:M]
constexpr uint16_t kWireConnectionSucceededFixedSize = 4;
constexpr uint16_t kWireConnectionSucceededMaxSize   = 4 + 256;

// ConnectionFailed: [PID:2][ReasonLen:2][Reason:M]
constexpr uint16_t kWireConnectionFailedFixedSize = 4;
constexpr uint16_t kWireConnectionFailedMaxSize   = 4 + 256;

// ── Session Management (0x01xx) ──────────────────────────────────────

// InquireGameSessions: [PID:2]
constexpr uint16_t kWireInquireGameSessionsSize = 2;

// GameSessionInfo: [PID:2][SessionID:2][PlayerCount:1][NameLen:1][Name:N][State:1]
constexpr uint16_t kWireGameSessionInfoFixedSize = 7; // PID + SessionID + PlayerCount + NameLen + State
constexpr uint16_t kWireGameSessionInfoMaxSize   = 7 + 64;

// ── Gameplay (0x02xx) ────────────────────────────────────────────────

// Move: [PID:2][From:4][To:4]
constexpr uint16_t kWireMoveSize = 10;

// Resign: [PID:2]
constexpr uint16_t kWireResignSize = 2;

// PawnStateChange: [PID:2][Locator:4][NewState:1][PromoPiece:1]
constexpr uint16_t kWirePawnStateChangeSize = 8;

// ── State Verification (0x03xx) ──────────────────────────────────────

// VerifyGameState: [PID:2][PawnCount:1][PawnLocators:4*P]
constexpr uint16_t kWireVerifyGameStateFixedSize = 3;
constexpr uint16_t kWireVerifyGameStateMaxSize   = 3 + 256;

// ── Broadcast / State Sync (0x04xx) ──────────────────────────────────

// RequestGameState: [PID:2]
constexpr uint16_t kWireRequestGameStateSize = 2;

// GameState: [PID:2][FENLen:2][FEN:F]
constexpr uint16_t kWireGameStateFixedSize = 4;
constexpr uint16_t kWireGameStateMaxSize   = 4 + 512;

} // namespace protocol::wire
