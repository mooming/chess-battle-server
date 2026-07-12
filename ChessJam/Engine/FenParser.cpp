#include "FenParser.h"

#include <cstring>
#include <string>

namespace engine
{

// Piece character to encoding mapping:
//   r=rook, n=knight, b=bishop, q=queen, k=king, p=pawn (black)
//   R,N,B,Q,K,P (white)
static uint8_t charToPiece(char c)
{
    switch (c)
    {
        case 'r': return Board::encode(Piece::Rook,    Color::Black);
        case 'n': return Board::encode(Piece::Knight,  Color::Black);
        case 'b': return Board::encode(Piece::Bishop,  Color::Black);
        case 'q': return Board::encode(Piece::Queen,   Color::Black);
        case 'k': return Board::encode(Piece::King,    Color::Black);
        case 'p': return Board::encode(Piece::Pawn,    Color::Black);
        case 'R': return Board::encode(Piece::Rook,    Color::White);
        case 'N': return Board::encode(Piece::Knight,  Color::White);
        case 'B': return Board::encode(Piece::Bishop,  Color::White);
        case 'Q': return Board::encode(Piece::Queen,   Color::White);
        case 'K': return Board::encode(Piece::King,    Color::White);
        case 'P': return Board::encode(Piece::Pawn,    Color::White);
        default:  return 0;
    }
}

static char pieceToChar(uint8_t encoded)
{
    Piece  p = static_cast<Piece>((encoded >> 4) & 0x0F);
    Color  c = static_cast<Color>(encoded & 0x0F);

    if (c == Color::White)
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
    else
    {
        switch (p)
        {
            case Piece::Pawn:    return 'p';
            case Piece::Knight:  return 'n';
            case Piece::Bishop:  return 'b';
            case Piece::Rook:    return 'r';
            case Piece::Queen:   return 'q';
            case Piece::King:    return 'k';
            default: return '?';
        }
    }
}

bool FenParser::parse(const char* fen, Board& board)
{
    if (!fen) return false;

    board.clear();

    const char* pos = fen;

    // 1. Parse piece placement (until first space).
    if (!parsePlacement(pos, board)) return false;

    // 2. Skip whitespace, parse active color.
    while (*pos == ' ') ++pos;
    if (*pos == 'w') board.activeColor = Color::White;
    else if (*pos == 'b') board.activeColor = Color::Black;
    else return false;

    // 3. Skip to end of line (rest of FEN fields ignored for now).
    // The remaining fields (castling, en passant, halfmove, fullmove)
    // are not used by our engine.

    return true;
}

bool FenParser::parsePlacement(const char*& fen, Board& board)
{
    // FEN format: rank 8 first (top), rank 1 last (bottom).
    // We parse into a temp array first, then reverse the rank order.
    uint8_t temp[64] = {};
    int sq = 0;
    while (*fen && *fen != ' ' && sq < 64)
    {
        if (*fen == '/')
        {
            ++fen;
            continue;
        }
        if (*fen >= '1' && *fen <= '8')
        {
            sq += (*fen - '0');
            ++fen;
            continue;
        }
        temp[sq++] = charToPiece(*fen++);
    }
    if (sq != 64) return false;

    // Now copy to board with correct rank ordering.
    // FEN group 0 (first in string) = rank 8 = board squares 56-63
    // FEN group 7 (last in string) = rank 1 = board squares 0-7
    for (int i = 0; i < 64; ++i)
    {
        int fenRank = i / 8;  // 0 = rank 8, 7 = rank 1
        int boardRank = 7 - fenRank;  // 7 = rank 8, 0 = rank 1
        int boardSq = boardRank * 8 + (i % 8);
        board.set(boardSq, temp[i]);
    }
    return true;
}

int FenParser::generatePlacement(const Board& board, char* buf, int bufSize)
{
    int pos = 0;
    // Standard FEN: rank 8 first (top, squares 56-63), rank 1 last (bottom, squares 0-7).
    for (int rank = 7; rank >= 0; --rank)
    {
        int empty = 0;
        for (int file = 0; file < 8; ++file)
        {
            int sq = rank * 8 + file;
            if (board.isEmpty(sq))
            {
                ++empty;
            }
            else
            {
                if (empty > 0)
                {
                    if (pos + 1 >= bufSize) return pos;
                    buf[pos++] = static_cast<char>('0' + empty);
                    empty = 0;
                }
                if (pos + 1 >= bufSize) return pos;
                buf[pos++] = pieceToChar(board.get(sq));
            }
        }
        if (empty > 0)
        {
            if (pos + 1 >= bufSize) return pos;
            buf[pos++] = static_cast<char>('0' + empty);
        }
        if (rank > 0)
        {
            if (pos + 1 >= bufSize) return pos;
            buf[pos++] = '/';
        }
    }
    return pos;
}

std::string FenParser::generate(const Board& board)
{
    char buf[256];
    int  len = generatePlacement(board, buf, sizeof(buf));
    buf[len] = '\0';

    std::string fen(buf, static_cast<size_t>(len));
    fen += " ";
    fen += (board.activeColor == Color::White) ? "w" : "b";
    return fen;
}

int FenParser::generate(const Board& board, char* buf, int bufSize)
{
    int len = generatePlacement(board, buf, bufSize - 4); // leave room for " w"
    buf[len++] = ' ';
    buf[len++] = (board.activeColor == Color::White) ? 'w' : 'b';
    buf[len] = '\0';
    return len;
}

} // namespace engine
