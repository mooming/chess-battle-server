#include "ClientStub.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

static void printUsage(const char* prog)
{
    std::cout << "Usage: " << prog << " <Server-Address> [SessionID]\n"
              << "  Server-Address: host:port (e.g., localhost:12345)\n"
              << "  SessionID: optional session to join (0 = create new)\n";
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string serverAddr = argv[1];
    uint16_t sessionId = 0;
    if (argc >= 3)
        sessionId = static_cast<uint16_t>(std::stoi(argv[2]));

    ClientStub client;
    client.connect(serverAddr, 12345);
    if (client.getFd() < 0)
    {
        std::cerr << "Failed to connect to " << serverAddr << std::endl;
        return 1;
    }

    // Send greeting
    client.sendGreeting("TestClient");

    // Receive server greeting
    std::string serverName, serverId;
    if (!client.receiveGreeting(serverName, serverId))
    {
        std::cerr << "Failed to receive server greeting" << std::endl;
        return 1;
    }
    std::cout << "Server: " << serverName << " (id=" << serverId << ")" << std::endl;

    // Receive connection succeeded
    std::string msg;
    if (!client.receiveConnectionSucceeded(msg))
    {
        std::cerr << "Connection failed: " << msg << std::endl;
        return 1;
    }
    std::cout << msg << std::endl;

    // Inquire sessions
    client.inquireSessions();

    // Receive session list
    bool hasSessions = false;
    uint16_t sid = 0;
    uint8_t players = 0;
    std::string name;
    uint8_t state = 0;

    while (client.receiveSessionInfo(sid, players, name, state))
    {
        hasSessions = true;
        std::cout << "Session " << sid << ": " << name
                  << " (" << (int)players << " players, state=" << (int)state << ")" << std::endl;

        // Auto-join first waiting session if none specified
        if (sessionId == 0 && state == 0 && players < 2)
        {
            std::cout << "Joining session " << sid << std::endl;
            sessionId = sid;
            break;
        }
    }

    if (!hasSessions)
    {
        std::cout << "No sessions available. Creating one..." << std::endl;
        // For now, we'll just note that session creation isn't implemented in the client stub
        // The server auto-creates sessions when the first client connects
    }

    // If we have a session, play a few moves
    if (sessionId > 0)
    {
        std::cout << "\nPlaying in session " << sessionId << "..." << std::endl;

        // Make a move: e2-e4 (WKD1 -> WPe3... wait, let me use correct locators)
        // Actually, the Move protocol uses PawnLocators: From (4 chars) + To (4 chars)
        // WKD1 = White King on D1, WPe2 = White Pawn on e2
        // Let's make e2-e4: WPe2 -> WPe4
        client.sendMove("WPe2", "WPe4");
        std::cout << "Played: e2-e4" << std::endl;

        // Request game state
        client.sendRequestGameState();
        std::string fen;
        if (client.receiveGameState(fen))
        {
            std::cout << "Board state: " << fen << std::endl;
        }

        // Send verify game state
        std::vector<std::string> pawns;
        // For simplicity, just send an empty pawn list (real client would compute this)
        client.sendVerifyState(pawns);
        std::cout << "Sent VerifyGameState" << std::endl;

        // Resign
        std::cout << "Resigning..." << std::endl;
        client.sendResign();

        // Receive final message
        std::string finalMsg;
        if (client.receiveConnectionSucceeded(finalMsg))
        {
            std::cout << finalMsg << std::endl;
        }
    }

    client.close();
    std::cout << "\nDisconnected." << std::endl;
    return 0;
}
