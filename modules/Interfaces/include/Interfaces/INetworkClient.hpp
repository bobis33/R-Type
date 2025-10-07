///
/// @file INetworkClient.hpp
/// @brief This file contains the client network interface
/// @namespace eng
///

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "Interfaces/Protocol/Protocol.hpp"
#include "Utils/Interfaces/IPlugin.hpp"

namespace eng
{

    ///
    /// @class INetworkClient
    /// @brief Interface for the client network
    /// @namespace eng
    ///
    class INetworkClient : public utl::IPlugin
    {
        public:
            using PacketHandler = std::function<void(const rnp::PacketHeader &, const std::vector<uint8_t> &)>;

            virtual ~INetworkClient() = default;

            // Connection management
            virtual void connect(const std::string &host, uint16_t port) = 0;
            virtual void disconnect() = 0;

            // Protocol messages
            virtual void sendConnect(const std::string &playerName) = 0;
            virtual void sendConnectWithCaps(const std::string &playerName, std::uint32_t clientCaps) = 0;
            virtual void sendDisconnect() = 0;
            virtual void sendDisconnect(rnp::DisconnectReason reason) = 0;
            virtual void sendPlayerInput(uint8_t direction, uint8_t shooting) = 0;
            virtual void sendPlayerInputAsEvent(std::uint16_t playerId, uint8_t direction, uint8_t shooting,
                                               uint32_t clientTimeMs) = 0;
            virtual void sendPing() = 0;
            virtual void sendPing(std::uint32_t nonce, std::uint32_t sendTimeMs) = 0;
            virtual void sendAck(std::uint32_t cumulative, std::uint32_t ackBits) = 0;

            // Handler management
            virtual void setPacketHandler(rnp::PacketType type, PacketHandler handler) = 0;
            virtual void setEventsHandler(std::function<void(const std::vector<rnp::EventRecord> &)> handler) = 0;

            // Getters
            virtual std::uint32_t getSessionId() const = 0;
            virtual std::uint16_t getServerTickRate() const = 0;

        private:
    }; // class INetworkClient

} // namespace eng