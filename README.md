# Chess Battle Server

This project is designed to provide a common server for multiple chess AI or player clients.
It offers public protocols, chess rules, and policies that every chess client should follow.

# Protocols

## Global State Protocols

Greeting (Server, Client) - Hello protocol to the server and from the server
* PID (UINT16) - Protocol ID, 0x0001
* Version (UINT16) - Entity Protocol Version
* Name (CHAR[64]) - Name of the entity
* IDLength (UINT16)
* ID (String) - Unique identifier issued by the server. It'll be used to continue the existing game.

ConnectionSucceeded (Server) - Indicating the connection has been agreed successfully.
* PID (UINT16) - Protocol ID, 0x0002
* MessageLength (UINT16)
* Message (String)

ConnectionFailed (Server) - Indicating the connection has been disagreed. 
* PID (UINT16) - Protocol ID, 0x0003
* ReasonLength (UINT16)
* Reason (String)

## Game Session Protocols

InquireGameSessions
GameSessionInfo

## Gameplay Protocols
Move (SERVER, CLIENT) - Indicating a movement of a pawn
* PID (UINT16) - Protocol ID
* PawnLocator (CHAR[4]) - A special code to denote a position of a pawn. e.g. WKD1(White King is placed at D1)

Resign

PawnStateChange

VerifyGameState - A protocol to verify game progress of each client. Every client should report the current gameplay state to ensure the game rule is valid for everyone. A client will be banned if it fails.
* PID (UINT16) - Protocol ID
* NumberOfPanws (UINT8) - It'll be used to calculate the length of pawn locators
* PawnLocators (String) - WKD1(White King is placed at D1)

## Broadcast Protocols
RequestGameState
GameState
