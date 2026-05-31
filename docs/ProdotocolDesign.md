# Chess Battle Server Protocol Design Document

## 1. Overview
The Chess Battle Server mediates communication between multiple chess‑AI or human clients. All interactions follow a binary, length‑prefixed message format built on top of TCP. Each message begins with a **Protocol ID (PID)** (16‑bit unsigned) that identifies the message type, followed by a payload whose layout is defined per‑message.

### Handshake (Greeting) Protocol
The handshake is a bidirectional exchange of **Greeting** messages (PID `0x0001`). Both client and server send a Greeting containing the protocol version and an entity name. After each side receives the peer's Greeting, it replies with **ConnectionSucceeded** (PID `0x0002`) to confirm the connection, or **ConnectionFailed** (PID `0x0003`) to abort. The handshake is considered **completed** only after both parties have exchanged `ConnectionSucceeded`. Only then may gameplay messages be exchanged.

The design is intentionally minimal to allow easy implementation in C++ (or any language) while being extensible for future features (e.g., tournament management, chat).

---

## 2. Message Framing
| Field | Size | Type | Description |
|-------|------|------|-------------|
| **PID** | 2 bytes | `uint16_t` (network byte order) | Identifies the message type. |
| **Length** (optional) | 2 bytes | `uint16_t` (network byte order) | Length of the remaining payload; present for all messages that carry variable‑size data. |
| **Payload** | variable | – | Message‑specific data. |

All multi‑byte integers are transmitted in **big‑endian** (network) order.

---

## 3. Core Protocol IDs

### Message Structure Overview
Below each protocol ID is described with field sizes (in bytes) and a brief description.

| PID (hex) | Message | Structure (bytes) |
|-----------|---------|-------------------|
| `0x0001` | **Greeting** | 2 (PID) + 2 (Version) + 2 (NameLen) + N (Name) |
| `0x0002` | **ConnectionSucceeded** | 2 (PID) + 2 (MsgLen) + M (Message) |
| `0x0003` | **ConnectionFailed** | 2 (PID) + 2 (ReasonLen) + R (Reason) |
| `0x0100` | **InquireGameSessions** | 2 (PID) (no payload) |
| `0x0101` | **GameSessionInfo** | 2 (PID) + 2 (SessionID) + 1 (PlayerCount) + 1 (NameLen) + N (Name) + 1 (State) |
| `0x0200` | **Move** | 2 (PID) + 4 (From) + 4 (To) |
| `0x0201` | **Resign** | 2 (PID) (no payload) |
| `0x0202` | **PawnStateChange** | 2 (PID) + variable (depends on change) |
| `0x0300` | **VerifyGameState** | 2 (PID) + 1 (PawnCount) + 4*P (PawnLocators) |
| `0x0400` | **RequestGameState** | 2 (PID) (no payload) |
| `0x0401` | **GameState** | 2 (PID) + 2 (FENLen) + F (FEN string) |

### Detailed Message Formats

#### 0x0001 Greeting
- **PID** (2 bytes): `0x0001`
- **Version** (2 bytes): protocol version number
- **NameLen** (2 bytes): length of the entity name in bytes
- **Name** (NameLen bytes): UTF‑8 entity name (e.g., "ChessClient")

#### 0x0002 ConnectionSucceeded
- **PID** (2 bytes): `0x0002`
- **MsgLen** (2 bytes): length of the welcome message
- **Message** (MsgLen bytes): UTF‑8 welcome text

#### 0x0003 ConnectionFailed
- **PID** (2 bytes): `0x0003`
- **ReasonLen** (2 bytes): length of the failure reason string
- **Reason** (ReasonLen bytes): UTF‑8 reason text

#### 0x0100 InquireGameSessions
- **PID** (2 bytes): `0x0100`
- No payload.

#### 0x0101 GameSessionInfo
- **PID** (2 bytes): `0x0101`
- **SessionID** (2 bytes): unique session identifier
- **PlayerCount** (1 byte): number of participants
- **NameLen** (1 byte): length of the session name
- **Name** (NameLen bytes): UTF‑8 session name
- **State** (1 byte): session state (0 = waiting, 1 = active, 2 = finished)

#### 0x0200 Move
- **PID** (2 bytes): `0x0200`
- **From** (4 bytes): source square code (e.g., "e2")
- **To** (4 bytes): destination square code (e.g., "e4")

#### 0x0201 Resign
- **PID** (2 bytes): `0x0201`
- No payload.

#### 0x0202 PawnStateChange
- **PID** (2 bytes): `0x0202`
- **Payload** (variable): details of promotion, capture, etc.

#### 0x0300 VerifyGameState
- **PID** (2 bytes): `0x0300`
- **PawnCount** (1 byte): number of pawn descriptors
- **PawnLocators** (4 × PawnCount bytes): concatenated pawn location strings

#### 0x0400 RequestGameState
- **PID** (2 bytes): `0x0400`
- No payload.

#### 0x0401 GameState
- **PID** (2 bytes): `0x0401`
- **FENLen** (2 bytes): length of the FEN string
- **FEN** (FENLen bytes): UTF‑8 FEN representation of the board

All multi‑byte integers are transmitted in **big‑endian** order.

| PID (hex) | Name | Direction | Description |
|-----------|------|-----------|-------------|
| `0x0001` | **Greeting** | Server → Client & Client → Server | Initial handshake containing protocol version and entity name. |
| `0x0002` | **ConnectionSucceeded** | Server → Client | Indicates that the client has been accepted. |
| `0x0003` | **ConnectionFailed** | Server → Client | Indicates rejection with a textual reason. |
| `0x0100` | **InquireGameSessions** | Client → Server | Request a list of active game sessions. |
| `0x0101` | **GameSessionInfo** | Server → Client | Response containing details (session ID, participants, state). |
| `0x0200` | **Move** | Server ↔ Client | Carries a pawn/figure movement. |
| `0x0201` | **Resign** | Client → Server | Signals that the player resigns. |
| `0x0202` | **PawnStateChange** | Server → Client | Notifies of a pawn’s promotion or capture. |
| `0x0300` | **VerifyGameState** | Client → Server | Client reports its current board state for validation. |
| `0x0400` | **RequestGameState** | Client → Server | Requests a full snapshot of the current game. |
| `0x0401` | **GameState** | Server → Client | Sends the full board representation. |

> **Note:** IDs are grouped by high‑order byte to simplify future expansion (e.g., `0x05xx` could be used for chat).

---

## 4. Payload Formats
### 4.1 Greeting (`0x0001`)
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0      | 2    | `uint16_t` | Protocol version (e.g., `0x0001`). |
| 2      | 2    | `uint16_t` | Length of the entity name (`N`). |
| 4      | N    | bytes | UTF‑8 entity name (e.g., `"ChessServer"`). |
**Direction:** Server → Client (also accepted from client for symmetry).

### 4.2 ConnectionSucceeded (`0x0002`)
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0      | 2    | `uint16_t` | Length of the welcome message (`M`). |
| 2      | M    | bytes | UTF‑8 message (e.g., `"Welcome"`). |

### 4.3 ConnectionFailed (`0x0003`)
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0      | 2    | `uint16_t` | Length of the reason string (`R`). |
| 2      | R    | bytes | UTF‑8 reason (e.g., `"Invalid token"`). |

### 4.4 Move (`0x0200`)
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0      | 4    | `char[4]` | Pawn locator string (e.g., `"WKD1"`). |
| 4      | 4    | `char[4]` | Destination locator (optional, for full move). |
*The protocol can be extended to include promotion flags, castling, etc.*

### 4.5 VerifyGameState (`0x0300`)
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0      | 1    | `uint8_t` | Number of pawn descriptors (`P`). |
| 1      | variable | `char[4] * P` | Concatenated pawn locators (e.g., `"WKD1WBH2..."`). |
The client sends the exact list of pieces it believes are on the board. The server validates against its authoritative state and may respond with `ConnectionFailed` if mismatch is detected.

### 4.6 GameState (`0x0401`)
| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0      | 2    | `uint16_t` | Length of the FEN string (`F`). |
| 2      | F    | bytes | UTF‑8 FEN representation of the board. |

---

## 5. Session Management
- **InquireGameSessions** (`0x0100`) – Client sends an empty payload; the server replies with one or more **GameSessionInfo** messages.
- **GameSessionInfo** (`0x0101`) payload includes:
  - `uint16_t` Session ID
  - `uint8_t` Player count
  - `char[64]` Session name (null‑terminated or length‑prefixed)
  - `uint8_t` Current state (e.g., `0 = waiting`, `1 = active`, `2 = finished`)

Clients may request to join a session by sending a **Greeting** with a custom identifier that the server maps to a session internally.

---

## 6. Error Handling & Security
1. **Malformed Messages** – If the server detects an unexpected PID or payload length, it sends **ConnectionFailed** with a reason and closes the socket.
2. **Version Mismatch** – The server checks the version field in **Greeting**; mismatched versions cause a failure response.
3. **State Verification** – Periodic **VerifyGameState** messages are required. Failure to report or mismatched state results in a ban (implementation‑defined; typically a temporary disconnect).

All string fields are UTF‑8 and limited to 255 bytes to prevent buffer overflow.

---

## 7. Extensibility Guidelines
- **New Message Types** – Assign a fresh PID in an unused high‑order range (`0x05xx`, `0x06xx`, …) and document the payload format.
- **Versioning** – Increment the version number in **Greeting** whenever the wire format changes incompatibly.
- **Optional Fields** – Use a presence bitmap (e.g., a `uint8_t` flags field) at the start of a payload to indicate which optional sub‑fields follow.

---

## 8. Example Interaction (Simplified)
```
Client → Server: Greeting
  PID=0x0001, Version=0x0001, NameLen=9, Name="ChessClient"

Server → Client: Greeting
  PID=0x0001, Version=0x0001, NameLen=11, Name="ChessServer"

Server → Client: ConnectionSucceeded
  PID=0x0002, MsgLen=7, Msg="Welcome"

Client → Server: InquireGameSessions
  PID=0x0100, (no payload)

Server → Client: GameSessionInfo (repeated for each session)
  PID=0x0101, SessionID=1, Players=2, NameLen=6, Name="Match1", State=0

Client → Server: Move
  PID=0x0200, Locator="WKD1", Dest="WKD2"

Server → Client: PawnStateChange (if promotion, capture, etc.)
  PID=0x0202, Payload=...
```
---

## 9. Implementation Checklist
- [ ] Define all PID constants in a shared header (`Protocol.h`).
- [ ] Implement binary read/write helpers for `uint16_t`, `uint8_t`, and length‑prefixed strings.
- [ ] Build a **Server** class that:
  - Accepts connections.
  - Sends **Greeting** and **ConnectionSucceeded**.
  - Parses incoming messages based on PID.
  - Handles `InquireGameSessions`, `Move`, `VerifyGameState`, etc.
- [ ] Build a **Client** stub for testing (optional).
- [ ] Write unit tests for each payload serialization/deserialization routine.

---

*Prepared for immediate integration into the Chess Battle Server codebase.*