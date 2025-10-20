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
        INVALID_PAYLOAD = 1,
        UNAUTHORIZED_SESSION = 2,
        RATE_LIMITED = 3,
        INTERNAL_ERROR = 4
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
        POWERUP = 0x04,
        OBSTACLE = 0x05
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
    /// Total size: 16 bytes
    ///
    struct PacketHeader
    {
            std::uint8_t type;
            std::uint16_t length;
            std::uint16_t flags;
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

} // namespace rnp
