#pragma once

#include "Board.h"
#include "FenParser.h"
#include "Rules.h"

#include <string>
#include <vector>

namespace engine
{

// ── Game State ────────────────────────────────────────────────────────
// Manages the state of a single chess game.

enum class GameStatus : uint8_t
{
    Ongoing,
    WhiteWins,
    BlackWins,
    Draw,
    Resigned
};

// Convert a Piece enum to its standard notation character.
inline char pieceToChar(Piece p)
{
    switch (p)
    {
        case Piece::Pawn:    return 'P';
        case Piece::Knight:  return 'N';
        case Piece::Bishop:  return 'B';
        case Piece::Rook:    return 'R';
        case Piece::Queen:   return 'Q';
        case Piece::King:    return 'K';
        default: return '?';
    }
}

// Convert a PawnLocator string (e.g. "WKD1") to a square index.
// Returns -1 if invalid.
inline int locatorToSquare(const char* loc)
{
    if (!loc || loc[0] == '\0' || loc[1] == '\0' || loc[2] == '\0' || loc[3] == '\0')
        return -1;
    int file = loc[2] - 'A';
    int rank = loc[3] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return Board::square(file, rank);
}

// Convert a square index to a PawnLocator string.
// Writes up to 4 chars + null terminator.
inline void squareToLocator(int sq, char out[4])
{
    int f = sq % 8;
    int r = sq / 8;
    out[0] = '\0'; // placeholder, caller fills in color and piece
    out[2] = static_cast<char>('A' + f);
    out[3] = static_cast<char>('1' + r);
}

struct Game
{
    Board board;
    GameStatus status = GameStatus::Ongoing;
    int moveCount = 0;
    Color lastMoveColor = Color::None;
    std::vector<Move> moveHistory;

    void reset()
    {
        board.clear();
        FenParser::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
        status = GameStatus::Ongoing;
        moveCount = 0;
        lastMoveColor = Color::None;
        moveHistory.clear();
    }

    // Apply a move from source to destination square.
    // Returns true if the move was legal and applied.
    bool makeMove(int from, int to)
    {
        if (status != GameStatus::Ongoing) return false;
        Move move(from, to);
        if (!Rules::isLegal(board, move)) return false;

        uint8_t piece = board.get(from);
        board.set(from, 0);
        board.set(to, piece);

        // Promotion: pawn reaching last rank becomes queen.
        int rank = to / 8;
        if (board.pieceOf(to) == Piece::Pawn && (rank == 0 || rank == 7))
        {
            board.set(to, Board::encode(Piece::Queen, board.colorOf(to)));
        }

        moveHistory.push_back(move);
        moveCount++;
        lastMoveColor = board.activeColor;
        board.activeColor = (board.activeColor == Color::White) ? Color::Black : Color::White;

        // Check for game end.
        int end = Rules::checkGameEnd(board);
        if (end == 1) status = GameStatus::WhiteWins;
        else if (end == 2) status = GameStatus::BlackWins;
        else if (end == 3) status = GameStatus::Draw;

        return true;
    }

    // Apply a move specified by two PawnLocators (e.g. "WKD1" → "WPd2").
    bool makeMove(const std::string& from, const std::string& to)
    {
        int f = locatorToSquare(from.c_str());
        int t = locatorToSquare(to.c_str());
        if (f < 0 || t < 0) return false;
        return makeMove(f, t);
    }

    std::string getFen() const
    {
        return FenParser::generate(board);
    }

    // Get all non-empty squares as PawnLocator strings for VerifyGameState.
    void getPawnState(std::vector<std::string>& pawns) const
    {
        pawns.clear();
        for (int sq = 0; sq < 64; ++sq)
        {
            if (board.isEmpty(sq)) continue;
            char loc[5];
            loc[0] = (board.colorOf(sq) == Color::White) ? 'W' : 'B';
            loc[1] = pieceToChar(board.pieceOf(sq));
            int f = sq % 8;
            int r = sq / 8;
            loc[2] = static_cast<char>('A' + f);
            loc[3] = static_cast<char>('1' + r);
            loc[4] = '\0';
            pawns.emplace_back(loc);
        }
    }

    bool isGameOver() const { return status != GameStatus::Ongoing; }
};

} // namespace engine
