///
/// @file Protocol.hpp
/// @brief This file contains the network protocol
/// @namespace rnp
///

#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace rnp
{

    inline constexpr std::uint8_t PROTOCOL_VERSION = 1;
    inline constexpr std::size_t MAX_PAYLOAD = 512;
    inline constexpr std::uint16_t DEFAULT_PORT = 4567;

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
        ERROR = 0x06,
        ACK = 0x07,
        ENTITY_EVENT = 0x08,
        CONNECT_ACCEPT = 0x09,
        PLAYER_INPUT = 0x03  // Deprecated: use ENTITY_EVENT with INPUT type
    };

    ///
    /// @brief Packet flags for reliability and fragmentation
    ///
    enum class PacketFlags : std::uint16_t
    {
        NONE = 0x0000,
        ACK_REQ = 0x0001,
        RELIABLE = 0x0002,
        FRAG = 0x0004,
        COMPRESSED = 0x0008
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
        std::uint8_t type;           // PacketType
        std::uint16_t length;        // Payload length in bytes
        std::uint16_t flags;         // PacketFlags bitfield
        std::uint16_t reserved;      // Must be 0
        std::uint32_t sequence;      // Per-session, monotonic sequence number
        std::uint32_t sessionId;     // Server-assigned session ID
    };

    ///
    /// @brief CONNECT packet payload
    ///
    struct PacketConnect
    {
        std::uint8_t nameLen;
        char playerName[32];
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
        std::uint16_t reasonCode;  // DisconnectReason
    };

    ///
    /// @brief Entity state for WORLD_STATE packet
    ///
    struct EntityState
    {
        std::uint32_t id;
        std::uint16_t type;        // EntityType
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
    /// @brief ACK packet payload
    ///
    struct PacketAck
    {
        std::uint32_t cumulativeAck;
        std::uint32_t ackBits;  // 32-bit SACK window
    };

    ///
    /// @brief ERROR packet payload
    ///
    struct PacketError
    {
        std::uint16_t errorCode;   // ErrorCode
        std::uint16_t msgLen;
        std::string description;
    };

    ///
    /// @brief Fragmentation header (when FRAG flag is set)
    ///
    struct FragmentHeader
    {
        std::uint16_t fragId;
        std::uint16_t fragIndex;
        std::uint16_t fragCount;
    };

    ///
    /// @brief Serialize events in ENTITY_EVENT format (TLV with entity_id)
    /// Format per event: type(1) | entity_id(4, BE) | data_len(1) | data(data_len)
    ///
    inline std::vector<std::uint8_t> serializeEvents(const std::vector<EventRecord> &events)
    {
        std::vector<std::uint8_t> payload;
        payload.reserve(64);

        for (const auto &ev : events)
        {
            const std::uint8_t dataLen = static_cast<std::uint8_t>(ev.data.size());
            
            // Event type (1 byte)
            payload.push_back(static_cast<std::uint8_t>(ev.type));
            
            // Entity ID (4 bytes, big endian)
            payload.push_back(static_cast<std::uint8_t>((ev.entityId >> 24) & 0xFF));
            payload.push_back(static_cast<std::uint8_t>((ev.entityId >> 16) & 0xFF));
            payload.push_back(static_cast<std::uint8_t>((ev.entityId >> 8) & 0xFF));
            payload.push_back(static_cast<std::uint8_t>(ev.entityId & 0xFF));
            
            // Data length (1 byte)
            payload.push_back(dataLen);
            
            // Data (dataLen bytes)
            payload.insert(payload.end(), ev.data.begin(), ev.data.end());

            if (payload.size() > MAX_PAYLOAD)
            {
                throw std::runtime_error("Events payload exceeds MAX_PAYLOAD");
            }
        }
        return payload;
    }

    ///
    /// @brief Deserialize ENTITY_EVENT payload into event records
    /// Format per event: type(1) | entity_id(4, BE) | data_len(1) | data(data_len)
    ///
    inline std::vector<EventRecord> deserializeEvents(const std::uint8_t *payload, const std::size_t length)
    {
        std::vector<EventRecord> events;
        std::size_t offset = 0;

        while (offset < length)
        {
            if (length - offset < 6)  // type(1) + entity_id(4) + data_len(1)
            {
                throw std::runtime_error("Truncated event header in payload");
            }

            const EventType type = static_cast<EventType>(payload[offset]);
            const std::uint32_t entityId = 
                (static_cast<std::uint32_t>(payload[offset + 1]) << 24) |
                (static_cast<std::uint32_t>(payload[offset + 2]) << 16) |
                (static_cast<std::uint32_t>(payload[offset + 3]) << 8) |
                static_cast<std::uint32_t>(payload[offset + 4]);
            const std::uint8_t dataLen = payload[offset + 5];
            offset += 6;

            if (length - offset < dataLen)
            {
                throw std::runtime_error("Truncated event data in payload");
            }

            EventRecord rec{type, entityId, {}};
            if (dataLen > 0)
            {
                rec.data.insert(rec.data.end(), payload + offset, payload + offset + dataLen);
            }
            events.emplace_back(std::move(rec));
            offset += dataLen;
        }
        return events;
    }

    ///
    /// @brief Serialize packet header (Big Endian as per RNP spec)
    ///
    inline std::vector<uint8_t> serializeHeader(const PacketHeader &header)
    {
        std::vector<uint8_t> buffer(16);  // Fixed header size
        
        buffer[0] = header.type;
        
        // length (2 bytes, big endian)
        buffer[1] = static_cast<uint8_t>((header.length >> 8) & 0xFF);
        buffer[2] = static_cast<uint8_t>(header.length & 0xFF);
        
        // flags (2 bytes, big endian)
        buffer[3] = static_cast<uint8_t>((header.flags >> 8) & 0xFF);
        buffer[4] = static_cast<uint8_t>(header.flags & 0xFF);
        
        // reserved (2 bytes)
        buffer[5] = static_cast<uint8_t>((header.reserved >> 8) & 0xFF);
        buffer[6] = static_cast<uint8_t>(header.reserved & 0xFF);
        
        // sequence (4 bytes, big endian)
        buffer[7] = static_cast<uint8_t>((header.sequence >> 24) & 0xFF);
        buffer[8] = static_cast<uint8_t>((header.sequence >> 16) & 0xFF);
        buffer[9] = static_cast<uint8_t>((header.sequence >> 8) & 0xFF);
        buffer[10] = static_cast<uint8_t>(header.sequence & 0xFF);
        
        // sessionId (4 bytes, big endian)
        buffer[11] = static_cast<uint8_t>((header.sessionId >> 24) & 0xFF);
        buffer[12] = static_cast<uint8_t>((header.sessionId >> 16) & 0xFF);
        buffer[13] = static_cast<uint8_t>((header.sessionId >> 8) & 0xFF);
        buffer[14] = static_cast<uint8_t>(header.sessionId & 0xFF);
        
        buffer[15] = 0; // Padding to 16 bytes
        
        return buffer;
    }

    ///
    /// @brief Serialize packet with header and optional payload (Big Endian)
    ///
    inline std::vector<uint8_t> serialize(const PacketHeader &header, const uint8_t *payload = nullptr)
    {
        std::vector<uint8_t> buffer = serializeHeader(header);
        
        if (payload && header.length > 0)
        {
            buffer.insert(buffer.end(), payload, payload + header.length);
        }
        
        return buffer;
    }

    ///
    /// @brief Deserialize packet header (Big Endian)
    ///
    inline PacketHeader deserializeHeader(const uint8_t *data, const std::size_t size)
    {
        if (size < 16)
        {
            throw std::runtime_error("Buffer too small for header");
        }
        
        PacketHeader header;
        
        header.type = data[0];
        
        // length (2 bytes, big endian)
        header.length = (static_cast<std::uint16_t>(data[1]) << 8) |
                       static_cast<std::uint16_t>(data[2]);
        
        // flags (2 bytes, big endian)
        header.flags = (static_cast<std::uint16_t>(data[3]) << 8) |
                      static_cast<std::uint16_t>(data[4]);
        
        // reserved (2 bytes)
        header.reserved = (static_cast<std::uint16_t>(data[5]) << 8) |
                         static_cast<std::uint16_t>(data[6]);
        
        // sequence (4 bytes, big endian)
        header.sequence = (static_cast<std::uint32_t>(data[7]) << 24) |
                         (static_cast<std::uint32_t>(data[8]) << 16) |
                         (static_cast<std::uint32_t>(data[9]) << 8) |
                         static_cast<std::uint32_t>(data[10]);
        
        // sessionId (4 bytes, big endian)
        header.sessionId = (static_cast<std::uint32_t>(data[11]) << 24) |
                          (static_cast<std::uint32_t>(data[12]) << 16) |
                          (static_cast<std::uint32_t>(data[13]) << 8) |
                          static_cast<std::uint32_t>(data[14]);
        
        return header;
    }

} // namespace rnp