#pragma once

#include <cstdint>
#include <array>

namespace engine
{

// ── Board Representation ──────────────────────────────────────────────
// 64 squares, indexed 0-63 (rank 1 = squares 0-7, rank 8 = squares 56-63).
// File a = column 0, file h = column 7.
//
// Encoding: Piece enum (0-6) << 4 | Color enum (0-1)
// Empty square = 0

enum class Piece : uint8_t { None = 0, Pawn = 1, Knight = 2, Bishop = 3, Rook = 4, Queen = 5, King = 6, Count = 7 };
enum class Color : uint8_t { White, Black, None };

struct Board
{
    // Square indexing: 0=A1, 1=B1, ..., 7=H1, 8=A2, ..., 63=H8
    static constexpr int kSize = 64;

    // Piece at square. 0 = empty.
    uint8_t squares[kSize] = {};

    // Active color: White moves first.
    Color activeColor = Color::White;

    void clear()
    {
        std::memset(squares, 0, sizeof(squares));
        activeColor = Color::White;
    }

    uint8_t get(int sq) const { return squares[sq]; }
    void    set(int sq, uint8_t piece) { squares[sq] = piece; }

    Color colorOf(int sq) const
    {
        return static_cast<Color>(squares[sq] & 0x0F);
    }

    Piece pieceOf(int sq) const
    {
        return static_cast<Piece>((squares[sq] >> 4) & 0x0F);
    }

    bool isEmpty(int sq) const { return squares[sq] == 0; }

    // Convert (file, rank) to square index. a1=0, h8=63.
    static int square(int file, int rank) { return rank * 8 + file; }

    // Convert square index to (file, rank).
    static void toSquare(int sq, int& file, int& rank)
    {
        file = sq % 8;
        rank = sq / 8;
    }

    // Find the square of a specific piece for a given color.
    int findPiece(Piece piece, Color color) const
    {
        uint8_t target = (static_cast<uint8_t>(piece) << 4) |
                         static_cast<uint8_t>(color);
        for (int i = 0; i < kSize; ++i)
            if (squares[i] == target) return i;
        return -1;
    }

    // Encode piece + color into a single byte.
    static uint8_t encode(Piece p, Color c)
    {
        return (static_cast<uint8_t>(p) << 4) | static_cast<uint8_t>(c);
    }
};

} // namespace engine
