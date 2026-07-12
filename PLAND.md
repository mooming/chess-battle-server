# Chess Battle Server — Development Plan

## Scope

### In Scope
- Complete C++ server implementation
- Full chess game logic (rules, move validation, state tracking)
- All protocol PIDs defined in ProtocolDesign.md
- Game session lifecycle (create, join, end)
- VerifyGameState anti-cheat
- GameState broadcast
- Thread-per-client concurrency
- CMake cross-platform build system
- Basic test infrastructure (unit + integration)
- Client stub for testing

### Out of Scope
- Client AI implementation
- GUI / web interface
- Tournament/league management
- Persistent storage
- TLS encryption
- Mobile clients

### Protocol Resolutions
- `Move` uses two 4-char locators (`From` + `To`) — ProtocolDesign prevails
- Greeting includes an `ID` field issued by server after `ConnectionSucceeded`
- All string fields use `uint16_t` length prefix (max 65535, practical caps per field)
- Typo `NumberOfPanws` corrected to `NumberOfPawns`

---

## Phase 1: Protocol Design (Estimated: 2 hours)

Define the canonical wire format, constants, and data structures.

### Steps
1. Create `Protocol/Protocol.h` — all PID constants, message structs
2. Create `Protocol/WireFormat.h` — wire layout enums for each message
3. Create `Protocol/Types.h` — shared types (PawnLocator, SquareCode, etc.)
4. Resolve all inconsistencies between README and ProtocolDesign
5. Write protocol reference summary (updated README.md)

### Verification
- [ ] All PIDs are unique and follow the grouping convention
- [ ] Wire layouts match ProtocolDesign.md exactly
- [ ] No field size ambiguity (every field has explicit byte count)
- [ ] Protocol reference document is self-consistent

---

## Phase 2: Server Logic (Estimated: 6 hours)

Implement the chess engine and game management — no networking yet.

### Steps
1. Create `Engine/Board.h/cpp` — board representation (8x8 array or bitboard)
2. Create `Engine/Game.h/cpp` — game state, turn tracking, move validation
3. Create `Engine/Rules.h/cpp` — move legality for each piece type
4. Create `Engine/FenParser.h/cpp` — parse/generate FEN strings
5. Create `Session/SessionManager.h/cpp` — session lifecycle (create, join, end)
6. Create `Session/ClientContext.h/cpp` — per-client state tracking

### Verification
- [ ] All piece moves validated correctly (knight L-shape, bishop diagonal, etc.)
- [ ] Check detection works (king under attack)
- [ ] Checkmate/stalemate detection works
- [ ] FEN parse → board → FEN serialize round-trips correctly
- [ ] SessionManager handles create/join/empty/full correctly
- [ ] Unit tests for Engine and Session (see Phase 4 for test framework)

---

## Phase 3: Protocol Implementation (Estimated: 8 hours)

Wire the protocol to the server logic. Full TCP networking with thread-per-client.

### Steps
1. Create `Network/BinaryIO.h/cpp` — read/write uint8, uint16, length-prefixed strings, fixed arrays
2. Create `Network/Server.h/cpp` — replace skeleton with threaded server:
   - Bind/listen with SO_REUSEADDR
   - `std::thread` per accepted client
   - Graceful shutdown via signal handler
3. Implement handshake:
   - `0x0001 Greeting` — receive client greeting, send server greeting + ConnectionSucceeded with assigned ID
   - `0x0003 ConnectionFailed` — version mismatch, session full, etc.
4. Implement session protocols:
   - `0x0100 InquireGameSessions` → `0x0101 GameSessionInfo` list
   - Join a session by client ID
5. Implement gameplay protocols:
   - `0x0200 Move` — validate via Engine, apply, broadcast PawnStateChange if applicable
   - `0x0201 Resign` — end game, notify opponent
   - `0x0202 PawnStateChange` — broadcast to all session members
6. Implement verification/broadcast:
   - `0x0300 VerifyGameState` — compare against authoritative state, ban on mismatch
   - `0x0400 RequestGameState` → `0x0401 GameState` (FEN)
7. Create `Client/ClientStub.h/cpp` — minimal client for testing
8. Create `CMakeLists.txt` — build configuration

### Verification
- [ ] Server compiles and links with CMake
- [ ] `make Server` produces executable
- [ ] `make Client` produces test client executable
- [ ] `make Test` runs unit tests
- [ ] Server accepts connection, completes handshake, assigns ID
- [ ] Client can inquire sessions, join, make valid moves
- [ ] Invalid moves are rejected (game logic validates)
- [ ] VerifyGameState detects and disconnects mismatched state
- [ ] Resign ends game and notifies both clients
- [ ] Thread-per-client handles 2+ simultaneous connections

---

## Phase 4: Testing (Estimated: 3 hours)

### Steps
1. Set up minimal test framework (header-only, no external dependency):
   - Create `Tests/UnitTest.h` — simple assertion macros + test runner
2. Write engine tests:
   - Move legality for all piece types
   - Check/checkmate/stalemate detection
   - FEN round-trip
3. Write protocol tests:
   - Serialization/deserialization of each message type
   - Network byte order correctness
4. Write integration test:
   - Client connects, handshakes, inquires sessions, makes a move, verifies state

### Verification
- [ ] All unit tests pass
- [ ] Integration test passes
- [ ] Tests run via `ctest` or `make test`
- [ ] Test output shows pass/fail per test case

---

## File Structure (Target)

```
ChessJam/
├── CMakeLists.txt
├── Main.cpp                  (entry point, unchanged)
├── CommandLineInput.*        (unchanged)
├── ExecutionMode.*           (unchanged)
├── StringHelper.*            (unchanged)
├── Server.*                  (replaced with threaded version)
├── Protocol/
│   ├── Protocol.h            (PID constants, message structs)
│   ├── WireFormat.h          (wire layout definitions)
│   └── Types.h               (shared types)
├── Network/
│   ├── BinaryIO.h/cpp        (read/write helpers)
│   └── Server.h/cpp          (threaded TCP server)
├── Engine/
│   ├── Board.h/cpp           (board representation)
│   ├── Game.h/cpp            (game state, turn, move application)
│   ├── Rules.h/cpp           (move legality per piece)
│   └── FenParser.h/cpp       (FEN parse/serialize)
├── Session/
│   ├── SessionManager.h/cpp  (session lifecycle)
│   └── ClientContext.h/cpp   (per-client state)
├── Client/
│   └── ClientStub.h/cpp      (minimal test client)
└── Tests/
    ├── UnitTest.h            (header-only test framework)
    ├── TestEngine.cpp        (engine unit tests)
    ├── TestProtocol.cpp      (protocol serialization tests)
    └── TestIntegration.cpp   (client-server integration test)
```

---

## Commit Units

1. **`protocol: add protocol.h with PIDs, structs, and types`** — Phase 1 deliverable
2. **`engine: add board representation and FEN parser`** — Phase 2 part 1
3. **`engine: add move rules and game state logic`** — Phase 2 part 2
4. **`session: add session manager and client context`** — Phase 2 part 3
5. **`network: add binary I/O helpers`** — Phase 3 part 1
6. **`network: implement threaded TCP server with handshake`** — Phase 3 part 2
7. **`network: implement session and gameplay protocol handlers`** — Phase 3 part 3
8. **`network: implement verify/gamestate and client stub`** — Phase 3 part 4
9. **`build: add CMakeLists.txt`** — Phase 3 part 5
10. **`tests: add unit test framework and engine tests`** — Phase 4 part 1
11. **`tests: add protocol and integration tests`** — Phase 4 part 2

---

## Verification Summary

| Phase | Verification Method |
|-------|-------------------|
| 1. Protocol Design | Cross-reference wire layouts against ProtocolDesign.md; self-consistency check |
| 2. Server Logic | Unit tests for Board, Game, Rules, FenParser, SessionManager |
| 3. Protocol Implementation | Build succeeds (CMake); integration test (client → server → move → verify) |
| 4. Testing | All tests pass via `ctest`; code review of test coverage |
