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

    enum class PacketType : std::uint8_t
    {
        CONNECT = 0x01,
        DISCONNECT = 0x02,
        PLAYER_INPUT = 0x03,
        WORLD_STATE = 0x04,
        PING = 0x05,
        PONG = 0x06,
        ERROR = 0x07,
        EVENTS = 0x08
    };

    ///
    /// @brief Type d'événements gameplay envoyés séparément du WORLD_STATE
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
    /// @brief Enregistrement d'événement au format TLV: type(1) | length(2, LE) | data(length)
    ///
    struct EventRecord
    {
        EventType type;
        std::vector<std::uint8_t> data;
    };

    ///
    /// @brief Sérialise une liste d'événements en payload binaire (TLV) respectant MAX_PAYLOAD
    ///
    inline std::vector<std::uint8_t> serializeEvents(const std::vector<EventRecord> &events)
    {
        std::vector<std::uint8_t> payload;
        payload.reserve(64);

        for (const auto &ev : events)
        {
            const std::uint16_t len = static_cast<std::uint16_t>(ev.data.size());
            // En-tête TLV: type (1)
            payload.push_back(static_cast<std::uint8_t>(ev.type));
            // length en little-endian (2)
            payload.push_back(static_cast<std::uint8_t>(len & 0xFF));
            payload.push_back(static_cast<std::uint8_t>((len >> 8) & 0xFF));
            // data (len)
            payload.insert(payload.end(), ev.data.begin(), ev.data.end());

            if (payload.size() > MAX_PAYLOAD)
            {
                throw std::runtime_error("Events payload exceeds MAX_PAYLOAD");
            }
        }
        return payload;
    }

    ///
    /// @brief Désérialise un payload d'événements (TLV) en liste d'enregistrements
    ///
    inline std::vector<EventRecord> deserializeEvents(const std::uint8_t *payload, const std::size_t length)
    {
        std::vector<EventRecord> events;
        std::size_t offset = 0;

        while (offset < length)
        {
            if (length - offset < 3)
            {
                throw std::runtime_error("Truncated event header in payload");
            }

            const EventType type = static_cast<EventType>(payload[offset]);
            const std::uint16_t len = static_cast<std::uint16_t>(payload[offset + 1]) |
                                      static_cast<std::uint16_t>(payload[offset + 2] << 8);
            offset += 3;

            if (length - offset < len)
            {
                throw std::runtime_error("Truncated event data in payload");
            }

            EventRecord rec{type, {}};
            if (len > 0)
            {
                rec.data.insert(rec.data.end(), payload + offset, payload + offset + len);
            }
            events.emplace_back(std::move(rec));
            offset += len;
        }
        return events;
    }

    struct PacketHeader
    {
            uint8_t version;
            PacketType type;
            uint16_t length;
            uint32_t sequence;
    };

    struct PacketConnect
    {
            PacketHeader header;
            char player_name[32];
    };

    inline std::vector<uint8_t> serialize(const PacketHeader &header, const uint8_t *payload = nullptr)
    {
        std::vector<uint8_t> buffer(sizeof(PacketHeader) + (payload ? header.length : 0));
        std::memcpy(buffer.data(), &header, sizeof(PacketHeader));
        if (payload && header.length > 0)
        {
            std::memcpy(buffer.data() + sizeof(PacketHeader), payload, header.length);
        }
        return buffer;
    }

    inline PacketHeader deserializeHeader(const uint8_t *data, const std::size_t size)
    {
        if (size < sizeof(PacketHeader))
        {
            throw std::runtime_error("Buffer too small for header");
        }
        PacketHeader header;
        std::memcpy(&header, data, sizeof(PacketHeader));
        return header;
    }

} // namespace rnp