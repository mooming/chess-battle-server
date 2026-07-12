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
