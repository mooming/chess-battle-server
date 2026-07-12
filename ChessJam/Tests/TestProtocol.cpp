#include "UnitTest.h"
#include "../Protocol/Protocol.h"
#include "../Protocol/WireFormat.h"

#include <cstring>
#include <iostream>

TEST(WireFormat_PIDs)
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

TEST(WireFormat_MoveSize)
{
    // Move: PID(2) + From(4) + To(4) = 10 bytes.
    ASSERT_EQ(protocol::wire::kWireMoveSize, 10);
    return true;
}

TEST(WireFormat_ResignSize)
{
    // Resign: PID(2) = 2 bytes.
    ASSERT_EQ(protocol::wire::kWireResignSize, 2);
    return true;
}

TEST(WireFormat_GameStateSize)
{
    // GameState: PID(2) + FENLen(2) + FEN(N) = 4 + N bytes.
    ASSERT_EQ(protocol::wire::kWireGameStateFixedSize, 4);
    ASSERT_TRUE(protocol::wire::kWireGameStateMaxSize >= 4);
    return true;
}

TEST(WireFormat_PawnStateChangeSize)
{
    // PawnStateChange: PID(2) + Locator(4) + NewState(1) + PromoPiece(1) = 8 bytes.
    ASSERT_EQ(protocol::wire::kWirePawnStateChangeSize, 8);
    return true;
}

TEST(WireFormat_InquireSize)
{
    // InquireGameSessions: PID(2) = 2 bytes.
    ASSERT_EQ(protocol::wire::kWireInquireGameSessionsSize, 2);
    return true;
}

TEST(WireFormat_RequestGameStateSize)
{
    // RequestGameState: PID(2) = 2 bytes.
    ASSERT_EQ(protocol::wire::kWireRequestGameStateSize, 2);
    return true;
}

TEST(WireFormat_ConnectionSucceededMaxSize)
{
    // ConnectionSucceeded: PID(2) + MsgLen(2) + Message(256) = 260 bytes max.
    ASSERT_EQ(protocol::wire::kWireConnectionSucceededMaxSize, 260);
    return true;
}

TEST(WireFormat_ConnectionFailedMaxSize)
{
    // ConnectionFailed: PID(2) + ReasonLen(2) + Reason(256) = 260 bytes max.
    ASSERT_EQ(protocol::wire::kWireConnectionFailedMaxSize, 260);
    return true;
}

TEST(WireFormat_VerifyGameStateMaxSize)
{
    // VerifyGameState: PID(2) + PawnCount(1) + PawnLocators(256) = 259 bytes max.
    ASSERT_EQ(protocol::wire::kWireVerifyGameStateMaxSize, 259);
    return true;
}

TEST(WireFormat_SessionInfoMaxSize)
{
    // GameSessionInfo: PID(2) + SessionID(2) + PlayerCount(1) + NameLen(1) + Name(64) + State(1) = 71 bytes max.
    ASSERT_EQ(protocol::wire::kWireGameSessionInfoMaxSize, 71);
    return true;
}

TEST(WireFormat_NoOverlappingPIDs)
{
    // All PIDs should be unique.
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

int main()
{
    std::cout << "=== Protocol Tests ===" << std::endl;
    return test::runAll();
}
