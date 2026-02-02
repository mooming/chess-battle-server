# Chess Battle Server

This project is designed to provide a common server for multiple chess AI or player clients.
It offers public protocols, chess rules, and policies that every chess client should follow.

# Protocols

## Global State Protocols

###Greeting
Version - Entity Protocol Version
EntityProfile - Entity Information; Name, ID, IP Address, Port Number

##IntegrationTestResult


###CreateGameSession
###GameSessions
###Connect
###Disconnect
###Reconnection
###ConnectionLost
###ConnectionTimedOut
###PlayerProfile
isHuman
isAI

###GameSessionProfile

## Game State Protocols


## Gameplay Protocols
Move
Resign

## Broadcast Protocols
GameState
