#include "Rules.h"

#include <algorithm>

namespace engine
{

// ── Helper: check if a direction delta is valid for the board ─────────

static bool onBoard(int sq) { return sq >= 0 && sq < 64; }
static int  fileOf(int sq) { return sq % 8; }
static int  rankOf(int sq) { return sq / 8; }

// Direction deltas for sliding pieces (file_delta, rank_delta).
static const int kDir4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };  // Rook/Queen
static const int kDir4d[4][2] = { { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } }; // Bishop/Queen
static const int kDir8[8][2] = {
    { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
    { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
}; // King

// Knight move offsets.
static const int kKnightDx[8] = { 1, 1, 2, 2, -1, -1, -2, -2 };
static const int kKnightDy[8] = { 2, -2, 1, -1, 2, 2, -1, -1 };

// ── Pseudo-legal move generation per piece type ───────────────────────

void Rules::generatePawnMoves(const Board& board, int from, std::vector<Move>& moves)
{
    Color color = board.colorOf(from);
    bool  isWhite = (color == Color::White);

    int dir = isWhite ? 1 : -1;  // rank direction
    int startRank = isWhite ? 1 : 6;
    int promoRank = isWhite ? 7 : 0;

    int f = fileOf(from);
    int r = rankOf(from);

    // Single push forward.
    int front = (r + dir) * 8 + f;
    if (onBoard(front) && board.isEmpty(front))
    {
        if (r + dir == promoRank)
        {
            // Promotion: generate all 4 promotion pieces.
            moves.emplace_back(from, front, Piece::Queen);
            moves.emplace_back(from, front, Piece::Rook);
            moves.emplace_back(from, front, Piece::Bishop);
            moves.emplace_back(from, front, Piece::Knight);
        }
        else
        {
            moves.emplace_back(from, front);
        }

        // Double push from starting rank.
        if (r == startRank)
        {
            int front2 = (r + 2 * dir) * 8 + f;
            if (onBoard(front2) && board.isEmpty(front2))
                moves.emplace_back(from, front2);
        }
    }

    // Diagonal captures.
    for (int df : { -1, 1 })
    {
        int cf = f + df;
        if (cf < 0 || cf > 7) continue;
        int capSq = (r + dir) * 8 + cf;
        if (!onBoard(capSq)) continue;
        if (board.isEmpty(capSq)) continue;
        if (board.colorOf(capSq) == color) continue; // own piece

        if (r + dir == promoRank)
        {
            moves.emplace_back(from, capSq, Piece::Queen);
            moves.emplace_back(from, capSq, Piece::Rook);
            moves.emplace_back(from, capSq, Piece::Bishop);
            moves.emplace_back(from, capSq, Piece::Knight);
        }
        else
        {
            moves.emplace_back(from, capSq);
        }
    }
}

void Rules::generateKnightMoves(const Board& board, int from, std::vector<Move>& moves)
{
    Color color = board.colorOf(from);
    int  f = fileOf(from);
    int  r = rankOf(from);

    for (int i = 0; i < 8; ++i)
    {
        int nf = f + kKnightDx[i];
        int nr = r + kKnightDy[i];
        if (nf < 0 || nf > 7 || nr < 0 || nr > 7) continue;
        int to = nr * 8 + nf;
        if (board.isEmpty(to) || board.colorOf(to) != color)
            moves.emplace_back(from, to);
    }
}

void Rules::generateSlidingMoves(const Board& board, int from,
                                  const int dirs[][2], int numDirs,
                                  std::vector<Move>& moves)
{
    Color color = board.colorOf(from);

    for (int d = 0; d < numDirs; ++d)
    {
        int df = dirs[d][0];
        int dr = dirs[d][1];
        int f = fileOf(from) + df;
        int r = rankOf(from) + dr;

        while (f >= 0 && f <= 7 && r >= 0 && r <= 7)
        {
            int to = r * 8 + f;
            if (board.isEmpty(to))
            {
                moves.emplace_back(from, to);
            }
            else
            {
                if (board.colorOf(to) != color)
                    moves.emplace_back(from, to);
                break; // blocked
            }
            f += df;
            r += dr;
        }
    }
}

void Rules::generateBishopMoves(const Board& board, int from, std::vector<Move>& moves)
{
    generateSlidingMoves(board, from, kDir4d, 4, moves);
}

void Rules::generateRookMoves(const Board& board, int from, std::vector<Move>& moves)
{
    generateSlidingMoves(board, from, kDir4, 4, moves);
}

void Rules::generateQueenMoves(const Board& board, int from, std::vector<Move>& moves)
{
    generateSlidingMoves(board, from, kDir4, 4, moves);
    generateSlidingMoves(board, from, kDir4d, 4, moves);
}

void Rules::generateKingMoves(const Board& board, int from, std::vector<Move>& moves)
{
    Color color = board.colorOf(from);
    int  f = fileOf(from);
    int  r = rankOf(from);

    for (int d = 0; d < 8; ++d)
    {
        int nf = f + kDir8[d][0];
        int nr = r + kDir8[d][1];
        if (nf < 0 || nf > 7 || nr < 0 || nr > 7) continue;
        int to = nr * 8 + nf;
        if (board.isEmpty(to) || board.colorOf(to) != color)
            moves.emplace_back(from, to);
    }
}

// ── Public API ────────────────────────────────────────────────────────

bool Rules::isPseudoLegal(const Board& board, const Move& move)
{
    if (!move.isValid()) return false;
    if (!onBoard(move.from) || !onBoard(move.to)) return false;
    if (board.isEmpty(move.from)) return false;
    if (board.colorOf(move.from) != board.activeColor) return false;
    if (!board.isEmpty(move.to) && board.colorOf(move.to) == board.activeColor)
        return false; // can't capture own piece

    Piece piece = board.pieceOf(move.from);
    std::vector<Move> genMoves;
    generateMoves(board, genMoves);

    for (const auto& m : genMoves)
    {
        if (m.from == move.from && m.to == move.to && m.promotion == move.promotion)
            return true;
    }
    return false;
}

bool Rules::isLegal(const Board& board, const Move& move)
{
    if (!isPseudoLegal(board, move)) return false;

    Board test = applyMove(board, move);
    // After the move, the opponent's turn. Check if their king is in check.
    // Actually: check if the moving side's king is still safe.
    Color movingColor = board.activeColor;
    Color oppColor = (movingColor == Color::White) ? Color::Black : Color::White;
    // The king that was moved belongs to movingColor. Find it and check if attacked.
    int kingSq = test.findPiece(Piece::King, movingColor);
    if (kingSq < 0) return false; // king captured (shouldn't happen)
    return !isSquareAttacked(test, kingSq, oppColor);
}

void Rules::generateMoves(const Board& board, std::vector<Move>& moves)
{
    moves.clear();
    for (int sq = 0; sq < 64; ++sq)
    {
        if (board.isEmpty(sq)) continue;
        if (board.colorOf(sq) != board.activeColor) continue;

        Piece p = board.pieceOf(sq);
        switch (p)
        {
            case Piece::Pawn:    generatePawnMoves(board, sq, moves); break;
            case Piece::Knight:  generateKnightMoves(board, sq, moves); break;
            case Piece::Bishop:  generateBishopMoves(board, sq, moves); break;
            case Piece::Rook:    generateRookMoves(board, sq, moves); break;
            case Piece::Queen:   generateQueenMoves(board, sq, moves); break;
            case Piece::King:    generateKingMoves(board, sq, moves); break;
            default: break;
        }
    }
}

bool Rules::isInCheck(const Board& board, Color color)
{
    int kingSq = board.findPiece(Piece::King, color);
    if (kingSq < 0) return true; // king missing = checkmate
    Color oppColor = (color == Color::White) ? Color::Black : Color::White;
    return isSquareAttacked(board, kingSq, oppColor);
}

bool Rules::hasLegalMoves(const Board& board, Color color)
{
    Board copy = board;
    copy.activeColor = color;
    std::vector<Move> moves;
    generateMoves(copy, moves);
    for (const auto& m : moves)
    {
        if (isLegal(copy, m)) return true;
    }
    return false;
}

int Rules::checkGameEnd(const Board& board)
{
    Color active = board.activeColor;
    if (isInCheck(board, active))
    {
        if (!hasLegalMoves(board, active))
            return 1; // checkmate — the side that just moved wins
    }
    else
    {
        if (!hasLegalMoves(board, active))
            return 2; // stalemate
    }
    return 0; // ongoing
}

bool Rules::isSquareAttacked(const Board& board, int sq, Color byColor)
{
    int sf = fileOf(sq);
    int sr = rankOf(sq);

    // Pawn attacks.
    {
        int pawnDir = (byColor == Color::White) ? -1 : 1; // attacking pawn comes from this direction
        for (int df : { -1, 1 })
        {
            int af = sf + df;
            int ar = sr + pawnDir;
            if (af < 0 || af > 7 || ar < 0 || ar > 7) continue;
            int atkSq = ar * 8 + af;
            if (board.pieceOf(atkSq) == Piece::Pawn && board.colorOf(atkSq) == byColor)
                return true;
        }
    }

    // Knight attacks.
    for (int i = 0; i < 8; ++i)
    {
        int af = sf + kKnightDx[i];
        int ar = sr + kKnightDy[i];
        if (af < 0 || af > 7 || ar < 0 || ar > 7) continue;
        int atkSq = ar * 8 + af;
        if (board.pieceOf(atkSq) == Piece::Knight && board.colorOf(atkSq) == byColor)
            return true;
    }

    // King attacks (adjacent).
    for (int d = 0; d < 8; ++d)
    {
        int af = sf + kDir8[d][0];
        int ar = sr + kDir8[d][1];
        if (af < 0 || af > 7 || ar < 0 || ar > 7) continue;
        int atkSq = ar * 8 + af;
        if (board.pieceOf(atkSq) == Piece::King && board.colorOf(atkSq) == byColor)
            return true;
    }

    // Sliding attacks: bishop/queen diagonals, rook/queen lines.
    // Diagonals (bishop + queen).
    for (int d = 0; d < 4; ++d)
    {
        int df = kDir4d[d][0];
        int dr = kDir4d[d][1];
        int af = sf + df;
        int ar = sr + dr;
        while (af >= 0 && af <= 7 && ar >= 0 && ar <= 7)
        {
            int atkSq = ar * 8 + af;
            if (!board.isEmpty(atkSq))
            {
                if (board.colorOf(atkSq) == byColor &&
                    (board.pieceOf(atkSq) == Piece::Bishop || board.pieceOf(atkSq) == Piece::Queen))
                    return true;
                break;
            }
            af += df;
            ar += dr;
        }
    }

    // Lines (rook + queen).
    for (int d = 0; d < 4; ++d)
    {
        int df = kDir4[d][0];
        int dr = kDir4[d][1];
        int af = sf + df;
        int ar = sr + dr;
        while (af >= 0 && af <= 7 && ar >= 0 && ar <= 7)
        {
            int atkSq = ar * 8 + af;
            if (!board.isEmpty(atkSq))
            {
                if (board.colorOf(atkSq) == byColor &&
                    (board.pieceOf(atkSq) == Piece::Rook || board.pieceOf(atkSq) == Piece::Queen))
                    return true;
                break;
            }
            af += df;
            ar += dr;
        }
    }

    return false;
}

Board Rules::applyMove(const Board& board, const Move& move)
{
    Board result = board;
    uint8_t piece = result.get(move.from);
    result.set(move.from, 0);

    // Only handle promotion if the moving piece is a pawn.
    if (board.pieceOf(move.from) == Piece::Pawn && move.promotion != Piece::Pawn)
    {
        Color c = board.colorOf(move.from);
        result.set(move.to, Board::encode(move.promotion, c));
    }
    else
    {
        result.set(move.to, piece);
    }

    return result;
}

} // namespace engine
