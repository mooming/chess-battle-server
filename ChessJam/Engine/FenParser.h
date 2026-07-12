#pragma once

#include "Board.h"

namespace engine
{

// ── FEN Parser / Generator ────────────────────────────────────────────
// Handles standard Forsyth-Edwards Notation:
//   [piece placement] [active color] [castling] [en passant]
// Example: "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3"

class FenParser
{
public:
    // Parse a FEN string into the board. Returns true on success.
    static bool parse(const char* fen, Board& board);

    // Generate a FEN string from the board. Writes up to 'bufSize' chars.
    // Returns the number of chars written (excluding null terminator).
    static int generate(const Board& board, char* buf, int bufSize);

    // Generate FEN into a std::string.
    static std::string generate(const Board& board);

private:
    static bool parsePlacement(const char*& fen, Board& board);
    static int  generatePlacement(const Board& board, char* buf, int bufSize);
};

} // namespace engine
