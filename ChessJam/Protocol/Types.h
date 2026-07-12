#pragma once

#include <cstdint>

namespace protocol
{

// Pawn locator: 4-char code identifying a piece and square.
// Format: <color><piece><file><rank>
//   Color: W=White, B=Black
//   Piece: K=King, Q=Queen, R=Rook, B=Bishop, N=Knight, P=Pawn
//   File:  A-H
//   Rank:  1-8
// Example: "WKD1" = White King on D1, "BPd5" = Black Pawn on d5
using PawnLocator = char[4];

// Square code: 2-char file + rank.
// Example: "e2", "d5"
using SquareCode = char[2];

// Entity name max length (bytes, excluding null terminator)
constexpr uint16_t kMaxEntityNameLen = 63;

// Session name max length (bytes, excluding null terminator)
constexpr uint16_t kMaxSessionNameLen = 63;

// Session ID max length
constexpr uint16_t kMaxSessionIdLen = 15;

} // namespace protocol
