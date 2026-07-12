#pragma once

#include "Board.h"

#include <vector>

namespace engine
{

// ── Move Definition ───────────────────────────────────────────────────

enum class MoveType : uint8_t { Normal, Capture, Promotion, Castling };

struct Move
{
    int from;   // Source square (0-63)
    int to;     // Destination square (0-63)
    Piece promotion; // For promotion moves (Pawn)

    Move() : from(-1), to(-1), promotion(Piece::Queen) {}
    Move(int f, int t, Piece p = Piece::Queen) : from(f), to(t), promotion(p) {}

    bool isValid() const { return from >= 0 && to >= 0 && from < 64 && to < 64; }
};

// ── Move Rules ────────────────────────────────────────────────────────
// Validates whether a move is legal for the given board state.

class Rules
{
public:
    // Check if a move is pseudo-legal (piece movement pattern is valid).
    // Does not check for king safety.
    static bool isPseudoLegal(const Board& board, const Move& move);

    // Check if a move is fully legal (pseudo-legal + doesn't leave king in check).
    static bool isLegal(const Board& board, const Move& move);

    // Generate all pseudo-legal moves for the active color.
    static void generateMoves(const Board& board, std::vector<Move>& moves);

    // Check if the active color's king is in check.
    static bool isInCheck(const Board& board, Color color);

    // Check if the active color has any legal moves (checkmate/stalemate detection).
    static bool hasLegalMoves(const Board& board, Color color);

    // Detect checkmate or stalemate. Returns:
    //   0 = game ongoing
    //   1 = checkmate (active color loses)
    //   2 = stalemate (draw)
    static int checkGameEnd(const Board& board);

    // Is the target square attacked by the given color?
    static bool isSquareAttacked(const Board& board, int sq, Color byColor);

private:
    // Per-piece move generators (pseudo-legal only).
    static void generatePawnMoves(const Board& board, int from, std::vector<Move>& moves);
    static void generateKnightMoves(const Board& board, int from, std::vector<Move>& moves);
    static void generateSlidingMoves(const Board& board, int from,
                                      const int dirs[][2], int numDirs,
                                      std::vector<Move>& moves);
    static void generateBishopMoves(const Board& board, int from, std::vector<Move>& moves);
    static void generateRookMoves(const Board& board, int from, std::vector<Move>& moves);
    static void generateQueenMoves(const Board& board, int from, std::vector<Move>& moves);
    static void generateKingMoves(const Board& board, int from, std::vector<Move>& moves);

    // Apply a move to a copy of the board (for legality checking).
    static Board applyMove(const Board& board, const Move& move);

    // Check if a square is on the board.
    static bool isOnBoard(int sq) { return sq >= 0 && sq < 64; }

    // Check if a square is on the correct file/rank.
    static bool isOnFile(int sq, int file) { return (sq % 8) == file; }
    static bool isOnRank(int sq, int rank) { return (sq / 8) == rank; }
};

} // namespace engine
