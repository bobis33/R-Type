///
/// @file Protocol.hpp
/// @brief This file contains the network protocol
/// @namespace rnp
///

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace rnp
{

    inline constexpr std::size_t MAX_PAYLOAD = 512;

    ///
    /// @brief Packet types according to RNP specification
    ///
    enum class PacketType : std::uint8_t
    {
        CONNECT = 0x01,
        DISCONNECT = 0x02,
        WORLD_STATE = 0x03,
        PING = 0x04,
        PONG = 0x05,
        PACKET_ERROR = 0x06,
        ENTITY_EVENT = 0x07,
        CONNECT_ACCEPT = 0x08,
        // Lobby system packets
        LOBBY_LIST_REQUEST = 0x09,
        LOBBY_LIST_RESPONSE = 0x0A,
        LOBBY_CREATE = 0x0B,
        LOBBY_CREATE_RESPONSE = 0x0C,
        LOBBY_JOIN = 0x0D,
        LOBBY_JOIN_RESPONSE = 0x0E,
        LOBBY_LEAVE = 0x0F,
        LOBBY_UPDATE = 0x10,
        GAME_START = 0x11,
        START_GAME_REQUEST = 0x12,
    };

    ///
    /// @brief Packet flags for reliability and fragmentation
    ///
    enum class PacketFlags : std::uint16_t
    {
        NONE = 0x0000,
        RELIABLE = 0x0001,
        COMPRESSED = 0x0002
    };

    ///
    /// @brief Disconnect reason codes
    ///
    enum class DisconnectReason : std::uint16_t
    {
        UNSPECIFIED = 0,
        CLIENT_REQUEST = 1,
        TIMEOUT = 2,
        PROTOCOL_ERROR = 3,
        SERVER_SHUTDOWN = 4,
        SERVER_FULL = 5,
        BANNED = 6
    };

    ///
    /// @brief Error codes
    ///
    enum class ErrorCode : std::uint16_t
    {
        NONE = 0,
        INVALID_PAYLOAD = 1,
        UNAUTHORIZED_SESSION = 2,
        RATE_LIMITED = 3,
        INTERNAL_ERROR = 4,
        LOBBY_NOT_FOUND = 5,
        LOBBY_FULL = 6,
        ALREADY_IN_LOBBY = 7,
        NOT_IN_LOBBY = 8,
        NOT_LOBBY_HOST = 9
    };

    ///
    /// @brief Event types for ENTITY_EVENT packets
    ///
    enum class EventType : std::uint8_t
    {
        SPAWN = 0x01,
        DESPAWN = 0x02,
        DAMAGE = 0x03,
        SCORE = 0x04,
        POWERUP = 0x05,
        INPUT = 0x06,
        CUSTOM = 0xFF
    };

    ///
    /// @brief Entity types for world state
    ///
    enum class EntityType : std::uint16_t
    {
        PLAYER = 0x01,
        ENEMY = 0x02,
        PROJECTILE = 0x03,
        BOSS = 0x04,
    };

    ///
    /// @brief Event record for ENTITY_EVENT packets (TLV format)
    ///
    struct EventRecord
    {
            EventType type;
            std::uint32_t entityId;
            std::vector<std::uint8_t> data;
    };

    ///
    /// @brief Packet header according to RNP specification (Big Endian)
    /// Total size: 7 bytes (1 + 2 + 4)
    ///
    struct PacketHeader
    {
            std::uint8_t type;
            std::uint16_t length;
            std::uint32_t sessionId;
    };

    ///
    /// @brief CONNECT packet payload
    ///
    struct PacketConnect
    {
            std::uint8_t nameLen;
            std::array<char, 32> playerName;
            std::uint32_t clientCaps;
    };

    ///
    /// @brief CONNECT_ACCEPT packet payload
    ///
    struct PacketConnectAccept
    {
            std::uint32_t sessionId;
            std::uint16_t tickRateHz;
            std::uint16_t mtuPayloadBytes;
            std::uint32_t serverCaps;
    };

    ///
    /// @brief DISCONNECT packet payload
    ///
    struct PacketDisconnect
    {
            std::uint16_t reasonCode; // DisconnectReason
    };

    ///
    /// @brief Entity state for WORLD_STATE packet
    ///
    struct EntityState
    {
            std::uint32_t id;
            std::uint16_t type; // EntityType
            float x, y;
            float vx, vy;
            std::uint8_t stateFlags;
    };

    ///
    /// @brief WORLD_STATE packet payload
    ///
    struct PacketWorldState
    {
            std::uint32_t serverTick;
            std::uint16_t entityCount;
            std::vector<EntityState> entities;
    };

    ///
    /// @brief PING/PONG packet payload
    ///
    struct PacketPingPong
    {
            std::uint32_t nonce;
            std::uint32_t sendTimeMs;
    };

    ///
    /// @brief ERROR packet payload
    ///
    struct PacketError
    {
            std::uint16_t errorCode; // ErrorCode
            std::uint16_t msgLen;
            std::string description;
    };

    ///
    /// @brief Lobby status enumeration
    ///
    enum class LobbyStatus : std::uint8_t
    {
        WAITING = 0,
        IN_GAME = 1,
        FINISHED = 2
    };

    ///
    /// @brief Lobby information structure
    ///
    struct LobbyInfo
    {
            std::uint32_t lobbyId;
            std::array<char, 32> lobbyName;
            std::uint8_t currentPlayers;
            std::uint8_t maxPlayers;
            std::uint8_t gameMode;
            std::uint8_t status; // LobbyStatus
            std::uint32_t hostSessionId;
            std::array<std::array<char, 32>, 8> playerNames; // Max 8 players
    };

    ///
    /// @brief LOBBY_LIST_RESPONSE packet payload
    ///
    struct PacketLobbyListResponse
    {
            std::uint16_t lobbyCount;
            std::vector<LobbyInfo> lobbies;
    };

    ///
    /// @brief LOBBY_CREATE packet payload
    ///
    struct PacketLobbyCreate
    {
            std::uint8_t nameLen;
            std::array<char, 32> lobbyName;
            std::uint8_t maxPlayers;
            std::uint8_t gameMode;
    };

    ///
    /// @brief LOBBY_CREATE_RESPONSE packet payload
    ///
    struct PacketLobbyCreateResponse
    {
            std::uint32_t lobbyId;
            std::uint8_t success;    // 0=failure, 1=success
            std::uint16_t errorCode; // ErrorCode if success=0
    };

    ///
    /// @brief LOBBY_JOIN packet payload
    ///
    struct PacketLobbyJoin
    {
            std::uint32_t lobbyId;
    };

    ///
    /// @brief LOBBY_JOIN_RESPONSE packet payload
    ///
    struct PacketLobbyJoinResponse
    {
            std::uint32_t lobbyId;
            std::uint8_t success;    // 0=failure, 1=success
            std::uint16_t errorCode; // ErrorCode if success=0
            LobbyInfo lobbyInfo;     // Current lobby state if success=1
    };

    ///
    /// @brief LOBBY_UPDATE packet payload
    ///
    struct PacketLobbyUpdate
    {
            LobbyInfo lobbyInfo;
    };

    ///
    /// @brief GAME_START packet payload
    ///
    struct PacketGameStart
    {
            std::uint32_t lobbyId;
    };

    ///
    /// @brief START_GAME_REQUEST packet payload (client requests to start game)
    ///
    struct PacketStartGameRequest
    {
            std::uint32_t lobbyId;
    };

} // namespace rnp
