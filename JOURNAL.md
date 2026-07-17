# Chess Battle Server — Journal

## 2026-07-11 — Project Init

### Context
- User requested a review of the README goals followed by a plan
- Existing codebase: VS solution with a TCP server skeleton, CLI routing to 5 modes (Monitor, Match, Judge, GameServer, GameClient)
- Detailed protocol spec exists in `docs/ProdotocolDesign.md` (11 PIDs across 4 layers)

### Decisions Made
1. **Language:** C++ (continue existing)
2. **Concurrency:** Thread-per-client (simple, portable, matches C++11 std::thread)
3. **Build system:** CMake (cross-platform, replaces VS-only .sln)
4. **Protocol inconsistencies resolved:**
   - `Move` uses From + To (4-char each) per ProtocolDesign
   - Greeting ID field added (per README mention)
   - String limits: uint16_t length prefix, practical caps per field
   - Typo fixed: NumberOfPanws → NumberOfPawns
5. **Priority order:** Protocol design → Server logic → Protocol implementation → Testing

### Open Questions
- Port number: default 12345 (from existing Server.cpp) — keep or change?
- Session join: by client ID or by session name? Current design says client ID
- Verification interval: how often should clients send VerifyGameState? (Not in protocol — implementation decision)

---

## 2026-07-12 — Implementation Complete

### What Was Built
- **Protocol layer** (`Protocol/`): PID constants, message structs, wire format constants, shared types
- **Network layer** (`Network/`): Binary I/O helpers for uint8/uint16/strings/fixed arrays (big-endian)
- **Engine layer** (`Engine/`): Board representation, FEN parser/generator, move rules (all piece types), game state management
- **Session layer** (`Session/`): SessionManager (create/join/leave/end sessions), ClientContext (per-client state)
- **Server** (`Server.cpp`): Thread-per-client TCP server with full protocol dispatch (handshake, sessions, moves, resign, verify, game state)
- **Tests** (`Tests/`): 20 unit tests covering Board, FEN, Rules, Game, Protocol — all passing
- **Build system**: CMakeLists.txt with Server, Tests targets

### Key Bugs Found & Fixed During Implementation
1. **FEN parse/generate rank reversal**: parsePlacement didn't reverse rank order; generatePlacement iterated in wrong direction
2. **Pawn encoding collision with empty square**: Piece::Pawn = 0 conflicted with empty. Fixed by adding Piece::None = 0
3. **applyMove treated all moves as promotions**: Default promotion was Piece::Queen, causing non-pawn moves to be mishandled. Fixed by checking if moving piece is actually a pawn
4. **const vector sort**: verifyGameState took const reference but needed to sort. Fixed by taking by value
5. **goto across functions**: Used return values instead of goto for error handling

### Remaining Work
- Client implementation (stub exists but no main)
- Integration test (client connects, handshakes, makes moves)
- PawnStateChange broadcast (skeleton in place, needs refinement)
- Verification interval scheduling (when to prompt clients for VerifyGameState)
- Connection ID reconnection (greeting ID field defined but reconnect logic incomplete)
- More comprehensive test coverage (edge cases, timeout handling, malformed messages)
