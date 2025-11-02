# R-Type Network Protocol (RNP) Specification

**Version:** 1.0
**Status:** Stable
**Last Updated:** 2025-01-XX

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Design Goals](#2-design-goals)
3. [Transport Layer](#3-transport-layer)
4. [Packet Structure](#4-packet-structure)
5. [Packet Types](#5-packet-types)
6. [Data Types and Encoding](#6-data-types-and-encoding)
7. [Connection Management](#7-connection-management)
8. [Lobby System](#8-lobby-system)
9. [Game State Synchronization](#9-game-state-synchronization)
10. [Entity System](#10-entity-system)
11. [Event System](#11-event-system)
12. [Error Handling](#12-error-handling)
13. [Timing and Latency](#13-timing-and-latency)
14. [Security Considerations](#14-security-considerations)
15. [Implementation Guidelines](#15-implementation-guidelines)

---

## 1. Introduction

The R-Type Network Protocol (RNP) is a UDP-based application protocol designed for real-time multiplayer gameplay in the R-Type game. It provides efficient, low-latency communication between game clients and servers with built-in reliability mechanisms, lobby management, and entity synchronization.

### 1.1 Protocol Namespace

This protocol operates in the `rnp` namespace to avoid conflicts with other network protocols.

---

## 2. Design Goals

- **Low Latency**: UDP-based for minimal overhead
- **Bandwidth Efficiency**: Compact binary encoding using network byte order
- **Reliability**: Application-level reliability for critical packets
- **Extensibility**: Versioned structures supporting future enhancements
- **Simplicity**: Clear separation of concerns between packet types
- **Scalability**: Support for multiple concurrent lobbies and game sessions

---

## 3. Transport Layer

### 3.1 Protocol Parameters

| Parameter          | Value           | Description                                  |
| ------------------ | --------------- | -------------------------------------------- |
| Transport Protocol | UDP             | Connectionless datagram protocol             |
| Default Port       | 4567            | Configurable server listening port           |
| Byte Order         | Big Endian      | Network byte order for all multi-byte values |
| Maximum Payload    | 512 bytes       | Maximum packet payload size                  |
| Rate Limit         | 200 packets/sec | Recommended per-client packet rate           |

### 3.2 Reliability

While UDP is inherently unreliable, RNP implements optional application-level reliability:

- Packets can be marked as `RELIABLE` via flags
- Critical packets (CONNECT, DISCONNECT, lobby operations) should use reliable delivery
- World state updates are typically unreliable (newer updates supersede older ones)
- Implementation-specific retransmission and acknowledgment mechanisms are recommended

---

## 4. Packet Structure

### 4.1 Packet Header

All RNP packets begin with a 7-byte header:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |            Length             |               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+               +
|                          Session ID                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Fields:**

- **Type** (1 byte): Packet type identifier (see Section 5)
- **Length** (2 bytes): Payload length in bytes (0-512)
- **Session ID** (4 bytes): Server-assigned session identifier (0 = not assigned)

### 4.2 Packet Flags

Optional flags may be implemented for packet control:

| Flag       | Value  | Description                                |
| ---------- | ------ | ------------------------------------------ |
| NONE       | 0x0000 | No special handling                        |
| RELIABLE   | 0x0001 | Requires acknowledgment and retransmission |
| COMPRESSED | 0x0002 | Payload is compressed                      |

_Note: Flag implementation is optional but recommended for production systems._

---

## 5. Packet Types

### 5.1 Type Enumeration

| Type ID | Name                  | Direction       | Description                       |
| ------- | --------------------- | --------------- | --------------------------------- |
| 0x01    | CONNECT               | Client → Server | Initial connection request        |
| 0x02    | DISCONNECT            | Bidirectional   | Connection termination            |
| 0x03    | WORLD_STATE           | Server → Client | Complete game state snapshot      |
| 0x04    | PING                  | Bidirectional   | Latency measurement request       |
| 0x05    | PONG                  | Bidirectional   | Latency measurement response      |
| 0x06    | PACKET_ERROR          | Server → Client | Error notification                |
| 0x07    | ENTITY_EVENT          | Bidirectional   | Entity-specific events            |
| 0x08    | CONNECT_ACCEPT        | Server → Client | Connection acknowledgment         |
| 0x09    | LOBBY_LIST_REQUEST    | Client → Server | Request list of available lobbies |
| 0x0A    | LOBBY_LIST_RESPONSE   | Server → Client | List of available lobbies         |
| 0x0B    | LOBBY_CREATE          | Client → Server | Create new lobby                  |
| 0x0C    | LOBBY_CREATE_RESPONSE | Server → Client | Lobby creation result             |
| 0x0D    | LOBBY_JOIN            | Client → Server | Join existing lobby               |
| 0x0E    | LOBBY_JOIN_RESPONSE   | Server → Client | Join result and lobby state       |
| 0x0F    | LOBBY_LEAVE           | Client → Server | Leave current lobby               |
| 0x10    | LOBBY_UPDATE          | Server → Client | Lobby state change notification   |
| 0x11    | GAME_START            | Server → Client | Game session start notification   |
| 0x12    | START_GAME_REQUEST    | Client → Server | Request to start game (host only) |
| 0x13    | GAME_OVER             | Server → Client | Game session end notification     |

---

## 6. Data Types and Encoding

### 6.1 Primitive Types

All multi-byte integers and floats use **Big Endian** (network byte order):

| Type    | Size    | Description                                  |
| ------- | ------- | -------------------------------------------- |
| uint8   | 1 byte  | Unsigned 8-bit integer                       |
| uint16  | 2 bytes | Unsigned 16-bit integer (big endian)         |
| uint32  | 4 bytes | Unsigned 32-bit integer (big endian)         |
| float32 | 4 bytes | IEEE 754 single-precision float (big endian) |

### 6.2 String Encoding

Strings use length-prefixed format:

```
uint8  length      // String length (0-255)
byte[] data        // UTF-8 encoded string data
```

### 6.3 Array Encoding

Arrays use count-prefixed format:

```
uint16 count       // Number of elements
...    elements    // Element data
```

---

## 7. Connection Management

### 7.1 Connection Handshake

```
Client                                Server
  |                                      |
  |--- CONNECT ----------------------->  |
  |    (playerName, clientCaps)          |
  |                                      |
  |<-- CONNECT_ACCEPT ------------------|
  |    (sessionId, tickRate, serverCaps) |
  |                                      |
```

### 7.2 CONNECT (0x01)

**Direction:** Client → Server

**Payload:**

```
uint8  nameLen              // Player name length (1-32)
byte[] playerName           // UTF-8 player name
uint32 clientCaps           // Client capability flags (reserved)
```

**Purpose:** Initiate connection and provide player identity.

### 7.3 CONNECT_ACCEPT (0x08)

**Direction:** Server → Client

**Payload:**

```
uint32 sessionId            // Assigned session identifier
uint16 tickRateHz           // Server tick rate (e.g., 50 Hz)
uint16 mtuPayloadBytes      // Maximum payload size (typically 512)
uint32 serverCaps           // Server capability flags (reserved)
```

**Purpose:** Acknowledge connection and provide session parameters.

### 7.4 DISCONNECT (0x02)

**Direction:** Bidirectional

**Payload:**

```
uint16 reasonCode           // Disconnect reason (see below)
```

**Disconnect Reasons:**

| Code | Name            | Description                 |
| ---- | --------------- | --------------------------- |
| 0    | UNSPECIFIED     | Generic disconnect          |
| 1    | CLIENT_REQUEST  | Client-initiated disconnect |
| 2    | TIMEOUT         | Connection timeout          |
| 3    | PROTOCOL_ERROR  | Protocol violation          |
| 4    | SERVER_SHUTDOWN | Server shutting down        |
| 5    | SERVER_FULL     | Server at capacity          |
| 6    | BANNED          | Client is banned            |

### 7.5 Keepalive

- Clients should send **PING** every 5 seconds during idle periods
- Servers should disconnect clients after 15 seconds without packets
- PING/PONG packets maintain session liveness

---

## 8. Lobby System

### 8.1 Lobby Lifecycle

```
Client                                Server
  |                                      |
  |--- LOBBY_LIST_REQUEST ------------>  |
  |<-- LOBBY_LIST_RESPONSE -------------|
  |                                      |
  |--- LOBBY_CREATE ------------------->|
  |<-- LOBBY_CREATE_RESPONSE -----------|
  |                                      |
  |--- LOBBY_JOIN --------------------->|
  |<-- LOBBY_JOIN_RESPONSE -------------|
  |<-- LOBBY_UPDATE --------------------|
  |                                      |
  |--- START_GAME_REQUEST ------------->|
  |<-- GAME_START ----------------------|
```

### 8.2 Lobby Status

| Code | Name     | Description                     |
| ---- | -------- | ------------------------------- |
| 0    | WAITING  | Lobby open, waiting for players |
| 1    | IN_GAME  | Game in progress                |
| 2    | FINISHED | Game completed                  |

### 8.3 Lobby Information Structure

```
uint32   lobbyId                // Unique lobby identifier
char[32] lobbyName              // Lobby name (null-terminated)
uint8    currentPlayers         // Current player count
uint8    maxPlayers             // Maximum allowed players (1-8)
uint8    gameMode               // Game mode identifier
uint8    status                 // Lobby status (see above)
uint32   hostSessionId          // Host player's session ID
char[32][8] playerNames         // Array of player names (up to 8)
```

### 8.4 LOBBY_LIST_REQUEST (0x09)

**Direction:** Client → Server

**Payload:** Empty

**Purpose:** Request list of all available lobbies.

### 8.5 LOBBY_LIST_RESPONSE (0x0A)

**Direction:** Server → Client

**Payload:**

```
uint16     lobbyCount           // Number of lobbies
LobbyInfo[] lobbies             // Array of lobby information
```

### 8.6 LOBBY_CREATE (0x0B)

**Direction:** Client → Server

**Payload:**

```
uint8    nameLen                // Lobby name length (1-32)
char[32] lobbyName              // Lobby name
uint8    maxPlayers             // Maximum players (1-8)
uint8    gameMode               // Game mode identifier
```

### 8.7 LOBBY_CREATE_RESPONSE (0x0C)

**Direction:** Server → Client

**Payload:**

```
uint32 lobbyId                  // Created lobby ID (0 if failed)
uint8  success                  // 1 = success, 0 = failure
uint16 errorCode                // Error code if success = 0
```

### 8.8 LOBBY_JOIN (0x0D)

**Direction:** Client → Server

**Payload:**

```
uint32 lobbyId                  // Target lobby ID
```

### 8.9 LOBBY_JOIN_RESPONSE (0x0E)

**Direction:** Server → Client

**Payload:**

```
uint32    lobbyId               // Lobby ID
uint8     success               // 1 = success, 0 = failure
uint16    errorCode             // Error code if success = 0
LobbyInfo lobbyInfo             // Current lobby state (if success = 1)
```

### 8.10 LOBBY_LEAVE (0x0F)

**Direction:** Client → Server

**Payload:** Empty (uses session ID from header)

### 8.11 LOBBY_UPDATE (0x10)

**Direction:** Server → Client (broadcast to lobby members)

**Payload:**

```
LobbyInfo lobbyInfo             // Updated lobby state
```

**Purpose:** Notify all lobby members of state changes (player join/leave, status change, etc.).

### 8.12 START_GAME_REQUEST (0x12)

**Direction:** Client → Server

**Payload:**

```
uint32 lobbyId                  // Lobby to start
```

**Purpose:** Host requests game start. Only valid from lobby host.

### 8.13 GAME_START (0x11)

**Direction:** Server → Client (broadcast to lobby members)

**Payload:**

```
uint32 lobbyId                  // Starting lobby ID
```

**Purpose:** Notify all players that the game is starting.

---

## 9. Game State Synchronization

### 9.1 WORLD_STATE (0x03)

**Direction:** Server → Client

**Payload:**

```
uint32        serverTick        // Current server tick number
uint16        entityCount       // Number of entities
EntityState[] entities          // Array of entity states
```

**Purpose:** Provide complete snapshot of game world state. Typically sent at server tick rate (e.g., 20-50 Hz).

### 9.2 Entity State Structure

```
uint32  id                      // Unique entity identifier
uint16  type                    // Entity type (see below)
uint8   subtype                 // Entity subtype/variant
float32 x                       // X position
float32 y                       // Y position
float32 vx                      // X velocity
float32 vy                      // Y velocity
uint8   healthPercent           // Health as percentage (0-100, 255=no health)
uint8   stateFlags              // Additional state flags
uint32  score                   // Player score (0 for non-players)
```

**Total Size:** 31 bytes per entity

---

## 10. Entity System

### 10.1 Entity Types

| Type ID | Name       | Description              |
| ------- | ---------- | ------------------------ |
| 0x01    | PLAYER     | Player-controlled entity |
| 0x02    | ENEMY      | Enemy entity             |
| 0x03    | PROJECTILE | Bullet/projectile entity |
| 0x04    | BOSS       | Boss enemy entity        |

### 10.2 Entity Subtypes

| Subtype ID | Name                           | Description                  |
| ---------- | ------------------------------ | ---------------------------- |
| 0          | NONE                           | No specific subtype          |
| 1          | ENEMY_BASIC                    | Basic enemy type             |
| 2          | ENEMY_ADVANCED                 | Advanced enemy type          |
| 3          | ENEMY_BOSS                     | Boss enemy type              |
| 10         | PROJECTILE_PLAYER              | Standard player projectile   |
| 11         | PROJECTILE_PLAYER_SUPERCHARGED | Powered-up player projectile |
| 12         | PROJECTILE_ENEMY               | Enemy projectile             |

### 10.3 State Flags

Implementation-specific flags for entity state. Example usage:

- Bit 0: Invulnerable
- Bit 1: Charging weapon
- Bit 2-7: Reserved

---

## 11. Event System

### 11.1 ENTITY_EVENT (0x07)

**Direction:** Bidirectional

**Payload:**

```
uint32         serverTick       // Server tick when events occurred
uint16         eventCount       // Number of events
EventRecord[]  events           // Array of events
```

### 11.2 Event Record Structure (TLV Format)

```
uint8  eventType                // Event type identifier
uint32 entityId                 // Target entity ID
uint8  dataLen                  // Event data length (0-255)
byte[] data                     // Event-specific data
```

### 11.3 Event Types

| Type ID | Name    | Description                   |
| ------- | ------- | ----------------------------- |
| 0x01    | SPAWN   | Entity spawned                |
| 0x02    | DESPAWN | Entity destroyed/removed      |
| 0x03    | DAMAGE  | Entity took damage            |
| 0x04    | SCORE   | Score update                  |
| 0x05    | POWERUP | Powerup collected             |
| 0x06    | INPUT   | Player input                  |
| 0xFF    | CUSTOM  | Implementation-specific event |

### 11.4 Event Data Formats

#### SPAWN (0x01)

```
uint16  entityType              // Type of spawned entity
float32 x                       // Spawn X position
float32 y                       // Spawn Y position
```

#### DESPAWN (0x02)

```
uint8 reason                    // Despawn reason code
```

#### DAMAGE (0x03)

```
uint16 amount                   // Damage amount
uint32 sourceId                 // Entity ID that caused damage
```

#### SCORE (0x04)

```
uint16 points                   // Points awarded
```

#### POWERUP (0x05)

```
uint16 powerupType              // Type of powerup collected
```

#### INPUT (0x06)

```
uint16 buttons                  // Button state bitfield
uint8  direction                // Movement direction (0-8, 0=none)
uint8  shooting                 // Shooting state (0=not shooting, 1=shooting)
uint32 clientTimeMs             // Client timestamp (for lag compensation)
```

---

## 12. Error Handling

### 12.1 PACKET_ERROR (0x06)

**Direction:** Server → Client

**Payload:**

```
uint16 errorCode                // Error code (see below)
uint16 msgLen                   // Description length
byte[] description              // UTF-8 error description
```

### 12.2 Error Codes

| Code | Name                 | Description                         |
| ---- | -------------------- | ----------------------------------- |
| 0    | NONE                 | No error                            |
| 1    | INVALID_PAYLOAD      | Malformed packet payload            |
| 2    | UNAUTHORIZED_SESSION | Invalid or expired session          |
| 3    | RATE_LIMITED         | Packet rate limit exceeded          |
| 4    | INTERNAL_ERROR       | Server internal error               |
| 5    | LOBBY_NOT_FOUND      | Requested lobby does not exist      |
| 6    | LOBBY_FULL           | Lobby at maximum capacity           |
| 7    | ALREADY_IN_LOBBY     | Client already in a lobby           |
| 8    | NOT_IN_LOBBY         | Operation requires lobby membership |
| 9    | NOT_LOBBY_HOST       | Operation requires host privileges  |

---

## 13. Timing and Latency

### 13.1 PING (0x04) / PONG (0x05)

**Direction:** Bidirectional

**Payload:**

```
uint32 nonce                    // Random identifier for matching pairs
uint32 sendTimeMs               // Sender's timestamp (milliseconds)
```

**Purpose:** Measure round-trip time (RTT) and maintain connection.

**Usage:**

1. Sender generates random nonce and records send time
2. Receiver echoes PING with PONG using same nonce
3. Sender calculates RTT from PONG receive time

### 13.2 Tick Rate

- Server tick rate is negotiated during CONNECT_ACCEPT
- Typical values: 20-50 Hz
- WORLD_STATE packets sent at tick rate
- Client interpolation recommended for smooth rendering

### 13.3 Client-Side Prediction

Recommended for INPUT events:

1. Client sends INPUT immediately
2. Client predicts local state
3. Server validates and sends authoritative WORLD_STATE
4. Client reconciles prediction with server state

---

## 14. Security Considerations

### 14.1 Session Management

- Session IDs are server-assigned and unpredictable
- Session IDs bind to (IP address, port) tuple
- Servers must validate session ID on every packet
- Invalid session IDs should trigger DISCONNECT or PACKET_ERROR

### 14.2 Rate Limiting

- Implement per-client packet rate limiting
- Recommended: 200 packets/second maximum
- Exceed rate limit → PACKET_ERROR with RATE_LIMITED code

### 14.3 Input Validation

- Validate all payload lengths against packet bounds
- Reject packets with invalid structure
- Sanitize string inputs (names, descriptions)
- Validate entity IDs exist before operations

### 14.4 Denial of Service Protection

- Implement connection limits per IP
- Timeout inactive sessions (15 seconds recommended)
- Reject oversized packets
- Validate all fields before processing

---

## 15. Implementation Guidelines

### 15.1 Byte Order Conversion

All implementations must convert between host byte order and network byte order (big endian):

```
Network → Host: For received packets
Host → Network: For transmitted packets
```

### 15.2 Packet Processing Pipeline

1. **Receive**: Read UDP datagram
2. **Validate**: Check header structure and session ID
3. **Deserialize**: Parse payload based on packet type
4. **Process**: Execute game logic
5. **Respond**: Serialize and send response packets

### 15.3 Reliability Implementation

For packets marked RELIABLE:

- Assign sequence numbers
- Store in retransmission buffer
- Retransmit until acknowledged
- Implement exponential backoff (200ms → 1600ms)
- Maximum retry limit: 6 attempts

### 15.4 Buffer Management

- Pre-allocate serialization buffers (512 bytes)
- Use buffer pools to reduce allocation overhead
- Clear/reset buffers between packets

### 15.5 Error Recovery

- Log all PACKET_ERROR instances
- Disconnect on critical errors (protocol violation, auth failure)
- Gracefully handle transient errors (rate limit, network issues)

### 15.6 Testing Recommendations

- **Unit Tests**: Serialize/deserialize for all packet types
- **Integration Tests**: Complete connection handshake
- **Stress Tests**: High packet rates, many concurrent clients
- **Network Simulation**: Packet loss, latency, reordering

---

## Appendix A: Complete Packet Examples

### Example 1: CONNECT

**Hex Dump:**

```
01                    // Type: CONNECT
00 09                 // Length: 9 bytes
00 00 00 00           // Session ID: 0 (not assigned yet)
05                    // Name length: 5
45 6C 6C 69 6F 74     // "Elliot" (UTF-8)
00 00 00 01           // Client caps: 0x00000001
```

### Example 2: CONNECT_ACCEPT

**Hex Dump:**

```
08                    // Type: CONNECT_ACCEPT
00 0C                 // Length: 12 bytes
A1 B2 C3 D4           // Session ID: 0xA1B2C3D4
00 32                 // Tick rate: 50 Hz
01 FC                 // MTU: 508 bytes
00 00 00 FF           // Server caps: 0x000000FF
```

### Example 3: ENTITY_EVENT (INPUT)

**Hex Dump:**

```
07                    // Type: ENTITY_EVENT
00 1C                 // Length: 28 bytes
12 34 56 78           // Session ID: 0x12345678
00 00 04 D2           // Server tick: 1234
00 01                 // Event count: 1
06                    // Event type: INPUT
00 00 00 2A           // Entity ID: 42
09                    // Data length: 9 bytes
00 01                 // Buttons: 0x0001
04                    // Direction: 4
01                    // Shooting: 1
00 00 DD D5           // Client time: 56789 ms
```

---

## Appendix B: Version History

| Version | Date       | Changes                                 |
| ------- | ---------- | --------------------------------------- |
| 1.0     | 2025-01-XX | Initial specification with lobby system |

---

## Appendix C: Reference Implementation

A reference implementation in C++ is available demonstrating:

- Big-endian serialization/deserialization
- Complete packet handler system
- Session management
- Lobby system
- Event processing

See the `rnp` namespace implementation for details.

---

**End of Specification**
