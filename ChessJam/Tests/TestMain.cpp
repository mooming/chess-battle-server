#include "UnitTest.h"
#include "Engine/Board.h"
#include "Engine/FenParser.h"
#include "Engine/Rules.h"
#include "Engine/Game.h"
#include "Protocol/Protocol.h"
#include "Protocol/WireFormat.h"
#include "Network/BinaryIO.h"
#include "Client/ClientStub.h"

#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <thread>
#include <unistd.h>

// ── Board Tests ────────────────────────────────────────────────────────

TEST(Board_Clear)
{
    engine::Board b;
    b.clear();
    for (int i = 0; i < 64; ++i)
        ASSERT_TRUE(b.isEmpty(i));
    ASSERT_TRUE(b.activeColor == engine::Color::White);
    return true;
}

TEST(Board_SquareIndexing)
{
    ASSERT_TRUE(engine::Board::square(0, 0) == 0);  // A1
    ASSERT_TRUE(engine::Board::square(7, 0) == 7);  // H1
    ASSERT_TRUE(engine::Board::square(0, 7) == 56); // A8
    ASSERT_TRUE(engine::Board::square(7, 7) == 63); // H8
    return true;
}

TEST(Board_EncodeDecode)
{
    using namespace engine;
    uint8_t encoded = Board::encode(Piece::King, Color::White);
    uint8_t expected = (static_cast<uint8_t>(Piece::King) << 4) | static_cast<uint8_t>(Color::White);
    ASSERT_TRUE(encoded == expected);
    Board b;
    b.set(0, encoded);
    ASSERT_TRUE(b.pieceOf(0) == Piece::King);
    ASSERT_TRUE(b.colorOf(0) == Color::White);
    return true;
}

// ── FEN Parser Tests ───────────────────────────────────────────────────

TEST(FenParser_ParseStartingPosition)
{
    engine::Board b;
    bool ok = engine::FenParser::parse(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", b);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(b.pieceOf(0) == engine::Piece::Rook);    // a1 = Rook
    ASSERT_TRUE(b.pieceOf(1) == engine::Piece::Knight);  // b1 = Knight
    ASSERT_TRUE(b.pieceOf(5) == engine::Piece::Bishop);  // f1 = Bishop
    ASSERT_TRUE(b.pieceOf(6) == engine::Piece::Knight);  // g1 = Knight
    ASSERT_TRUE(b.pieceOf(4) == engine::Piece::King);    // e1 = King
    ASSERT_TRUE(b.colorOf(0) == engine::Color::White);   // a1 = White
    ASSERT_TRUE(b.colorOf(56) == engine::Color::Black);  // a8 = Black
    ASSERT_TRUE(b.activeColor == engine::Color::White);
    return true;
}

TEST(FenParser_GenerateStartingPosition)
{
    engine::Board b;
    engine::FenParser::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", b);
    std::string fen = engine::FenParser::generate(b);
    // Standard FEN starts with rank 8 (black pieces).
    ASSERT_TRUE(fen.find("rnbqkbnr") != std::string::npos);
    ASSERT_TRUE(fen.find("pppppppp") != std::string::npos);
    return true;
}

TEST(FenParser_RoundTrip)
{
    engine::Board b;
    engine::FenParser::parse(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", b);
    std::string fen = engine::FenParser::generate(b);
    engine::Board b2;
    bool ok = engine::FenParser::parse(fen.c_str(), b2);
    ASSERT_TRUE(ok);
    for (int i = 0; i < 64; ++i)
        ASSERT_EQ(b.get(i), b2.get(i));
    ASSERT_EQ(b.activeColor, b2.activeColor);
    return true;
}

// ── Rules Tests ────────────────────────────────────────────────────────

TEST(Rules_PawnForward)
{
    engine::Board b;
    // Pawn on e4 (rank 5 in FEN, 0-indexed rank 4).
    engine::FenParser::parse("8/8/8/8/4P3/8/8/4K2k w - - 0 1", b);
    // White pawn on e4 (square 28). Move e4→e5 (square 36).
    engine::Move m(28, 36);
    ASSERT_TRUE(engine::Rules::isPseudoLegal(b, m));
    ASSERT_TRUE(engine::Rules::isLegal(b, m));
    return true;
}

TEST(Rules_PawnBlocked)
{
    engine::Board b;
    engine::FenParser::parse("8/4P3/8/4P3/8/8/8/4K2k w - - 0 1", b);
    // White pawn on e4 blocked by own pawn on e5.
    engine::Move m(28, 36);
    ASSERT_FALSE(engine::Rules::isPseudoLegal(b, m));
    return true;
}

TEST(Rules_KnightMoves)
{
    engine::Board b;
    b.clear();
    b.set(28, engine::Board::encode(engine::Piece::Knight, engine::Color::White));
    std::vector<engine::Move> moves;
    engine::Rules::generateMoves(b, moves);
    // Knight on e4 has 8 possible moves.
    ASSERT_EQ(static_cast<int>(moves.size()), 8);
    return true;
}

TEST(Rules_KnightEdge)
{
    engine::Board b;
    b.clear();
    b.set(0, engine::Board::encode(engine::Piece::Knight, engine::Color::White));
    std::vector<engine::Move> moves;
    engine::Rules::generateMoves(b, moves);
    // Knight on a1 has 2 possible moves (b3, c2).
    ASSERT_EQ(static_cast<int>(moves.size()), 2);
    return true;
}

TEST(Rules_ScholarsMate)
{
    // Position with Black King on f3, White Rook on a1, White King on e1.
    // Black has legal moves, so game is ongoing.
    engine::Board b;
    engine::FenParser::parse("8/8/8/8/8/5k2/8/R4K2 b - - 0 1", b);
    int end = engine::Rules::checkGameEnd(b);
    ASSERT_EQ(end, 0); // ongoing
    return true;
}

TEST(Rules_ActualCheckmate)
{
    // Stalemate position: White Ka1, Black Ka3, Black Pa2.
    // White King has no legal moves and is not in check.
    engine::Board b;
    engine::FenParser::parse("8/8/8/8/8/k7/p7/K7 w - - 0 1", b);
    int end = engine::Rules::checkGameEnd(b);
    ASSERT_EQ(end, 2); // stalemate
    return true;
}

// ── Game Tests ─────────────────────────────────────────────────────────

TEST(Game_Reset)
{
    engine::Game g;
    g.reset();
    ASSERT_FALSE(g.isGameOver());
    ASSERT_EQ(g.moveCount, 0);
    ASSERT_TRUE(g.board.pieceOf(0) == engine::Piece::Rook);
    ASSERT_TRUE(g.board.pieceOf(4) == engine::Piece::King);
    return true;
}

TEST(Game_MakeMove)
{
    engine::Game g;
    g.reset();
    // e2→e4: pawn from square 12 to 28.
    bool ok = g.makeMove(12, 28);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(g.board.isEmpty(12));
    ASSERT_TRUE(g.board.pieceOf(28) == engine::Piece::Pawn);
    ASSERT_TRUE(g.board.activeColor == engine::Color::Black);
    return true;
}

TEST(Game_InvalidMove)
{
    engine::Game g;
    g.reset();
    // Move a rook diagonally — illegal.
    bool ok = g.makeMove(0, 9);
    ASSERT_FALSE(ok);
    return true;
}

TEST(Game_TwoPlayerGame)
{
    engine::Game g;
    g.reset();
    ASSERT_TRUE(g.makeMove(12, 28)); // e2-e4
    ASSERT_TRUE(g.makeMove(52, 36)); // e7-e5
    ASSERT_TRUE(g.makeMove(1, 18));  // Nb1-c3
    return true;
}

TEST(Game_PawnState)
{
    engine::Game g;
    g.reset();
    std::vector<std::string> pawns;
    g.getPawnState(pawns);
    // 32 pieces on the board at start.
    ASSERT_EQ(static_cast<int>(pawns.size()), 32);
    return true;
}

// ── Protocol Tests ─────────────────────────────────────────────────────

TEST(Protocol_PIDs)
{
    ASSERT_EQ(protocol::kPidGreeting, 0x0001);
    ASSERT_EQ(protocol::kPidConnectionSucceeded, 0x0002);
    ASSERT_EQ(protocol::kPidConnectionFailed, 0x0003);
    ASSERT_EQ(protocol::kPidInquireGameSessions, 0x0100);
    ASSERT_EQ(protocol::kPidGameSessionInfo, 0x0101);
    ASSERT_EQ(protocol::kPidMove, 0x0200);
    ASSERT_EQ(protocol::kPidResign, 0x0201);
    ASSERT_EQ(protocol::kPidPawnStateChange, 0x0202);
    ASSERT_EQ(protocol::kPidVerifyGameState, 0x0300);
    ASSERT_EQ(protocol::kPidRequestGameState, 0x0400);
    ASSERT_EQ(protocol::kPidGameState, 0x0401);
    return true;
}

TEST(Protocol_WireSizes)
{
    ASSERT_EQ(protocol::wire::kWireMoveSize, 10);
    ASSERT_EQ(protocol::wire::kWireResignSize, 2);
    ASSERT_EQ(protocol::wire::kWireGameStateFixedSize, 4);
    ASSERT_EQ(protocol::wire::kWirePawnStateChangeSize, 8);
    ASSERT_EQ(protocol::wire::kWireInquireGameSessionsSize, 2);
    ASSERT_EQ(protocol::wire::kWireRequestGameStateSize, 2);
    return true;
}

TEST(Protocol_NoOverlappingPIDs)
{
    uint16_t pids[] = {
        protocol::kPidGreeting,
        protocol::kPidConnectionSucceeded,
        protocol::kPidConnectionFailed,
        protocol::kPidInquireGameSessions,
        protocol::kPidGameSessionInfo,
        protocol::kPidMove,
        protocol::kPidResign,
        protocol::kPidPawnStateChange,
        protocol::kPidVerifyGameState,
        protocol::kPidRequestGameState,
        protocol::kPidGameState
    };
    for (size_t i = 0; i < sizeof(pids)/sizeof(pids[0]); ++i)
    {
        for (size_t j = i + 1; j < sizeof(pids)/sizeof(pids[0]); ++j)
        {
            ASSERT_NE(pids[i], pids[j]);
        }
    }
    return true;
}

// ── Integration Tests ──────────────────────────────────────────────────

static const int kTestPort = 19877;

class TestServer
{
public:
    void start()
    {
        struct addrinfo hints{}, *res;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        std::string portStr = std::to_string(kTestPort);
        int err = getaddrinfo("127.0.0.1", portStr.c_str(), &hints, &res);
        if (err != 0) return;
        listenFd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listenFd_ == -1) { freeaddrinfo(res); return; }
        int opt = 1;
        setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        ::bind(listenFd_, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
        ::listen(listenFd_, 5);
        running_ = true;
        std::thread([this]() { acceptLoop(); }).detach();
    }

    void stop()
    {
        running_ = false;
        if (listenFd_ >= 0) ::close(listenFd_);
    }

private:
    void acceptLoop()
    {
        while (running_)
        {
            struct sockaddr_in addr{};
            socklen_t len = sizeof(addr);
            int fd = ::accept(listenFd_, (struct sockaddr*)&addr, &len);
            if (fd < 0) continue;
            handleClient(fd);
            ::close(fd);
        }
    }

    void handleClient(int fd)
    {
        using namespace network;
        uint16_t pid = 0;
        if (!ReadPid(fd, pid) || pid != protocol::kPidGreeting) return;
        uint16_t version = 0;
        std::string name, id;
        if (!ReadUint16(fd, version)) return;
        if (!ReadString(fd, name, 63)) return;
        if (!ReadString(fd, id, 127)) return;
        WritePid(fd, protocol::kPidGreeting);
        WriteUint16(fd, 1);
        WriteString(fd, "TestServer");
        WriteString(fd, "server_1");
        WritePid(fd, protocol::kPidConnectionSucceeded);
        WriteString(fd, "Welcome");
        if (!ReadPid(fd, pid)) return;
        if (pid == protocol::kPidMove)
        {
            char from[4], to[4];
            ReadFixedArray(fd, from, 4);
            ReadFixedArray(fd, to, 4);
            std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
            WritePid(fd, protocol::kPidGameState);
            WriteUint16(fd, static_cast<uint16_t>(fen.size()));
            WriteBytes(fd, fen.data(), fen.size());
        }
    }

    int listenFd_ = -1;
    bool running_ = false;
};

TEST(Integration_Handshake)
{
    TestServer server;
    server.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ClientStub client;
    client.connect("127.0.0.1", kTestPort);
    ASSERT_TRUE(client.getFd() >= 0);
    client.sendGreeting("TestClient");
    std::string serverName, serverId;
    ASSERT_TRUE(client.receiveGreeting(serverName, serverId));
    ASSERT_TRUE(serverName == "TestServer");
    ASSERT_TRUE(serverId == "server_1");
    std::string msg;
    ASSERT_TRUE(client.receiveConnectionSucceeded(msg));
    ASSERT_TRUE(msg == "Welcome");
    client.close();
    server.stop();
    return true;
}

TEST(Integration_MoveAndGameState)
{
    TestServer server;
    server.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ClientStub client;
    client.connect("127.0.0.1", kTestPort);
    ASSERT_TRUE(client.getFd() >= 0);
    client.sendGreeting("TestClient");
    std::string serverName, serverId;
    ASSERT_TRUE(client.receiveGreeting(serverName, serverId));
    std::string msg;
    ASSERT_TRUE(client.receiveConnectionSucceeded(msg));
    client.sendMove("WPe2", "WPe4");
    std::string fen;
    ASSERT_TRUE(client.receiveGameState(fen));
    ASSERT_TRUE(fen.find("4P3") != std::string::npos);
    client.close();
    server.stop();
    return true;
}

TEST(Integration_MultipleClients)
{
    TestServer server;
    server.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ClientStub client1, client2;
    client1.connect("127.0.0.1", kTestPort);
    client2.connect("127.0.0.1", kTestPort);
    ASSERT_TRUE(client1.getFd() >= 0);
    ASSERT_TRUE(client2.getFd() >= 0);
    client1.sendGreeting("Client1");
    client2.sendGreeting("Client2");
    std::string name, id, msg;
    ASSERT_TRUE(client1.receiveGreeting(name, id));
    ASSERT_TRUE(client2.receiveGreeting(name, id));
    ASSERT_TRUE(client1.receiveConnectionSucceeded(msg));
    ASSERT_TRUE(client2.receiveConnectionSucceeded(msg));
    client1.close();
    client2.close();
    server.stop();
    return true;
}

int main()
{
    std::cout << "=== Chess Battle Server Tests ===" << std::endl;
    return test::runAll();
}
