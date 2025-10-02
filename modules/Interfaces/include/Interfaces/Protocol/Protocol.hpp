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

    enum class PacketType : uint8_t
    {
        CONNECT = 0x01,
        DISCONNECT = 0x02,
        PLAYER_INPUT = 0x03,
        WORLD_STATE = 0x04,
        PING = 0x05,
        PONG = 0x06,
        ERROR = 0x07
    };

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