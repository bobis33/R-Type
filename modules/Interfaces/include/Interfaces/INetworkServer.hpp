///
/// @file INetworkServer.hpp
/// @brief This file contains the server network interface
/// @namespace srv
///

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Utils/Interfaces/IPlugin.hpp"

namespace srv
{

    constexpr size_t MAX_CLIENTS = 16;
    constexpr size_t MAX_IP_LENGTH = 8;
    constexpr size_t MAX_LEN_RECV_BUFFER = 1024;

    ///
    /// @class INetworkServer
    /// @brief Interface for the server network
    /// @namespace srv
    ///
    class INetworkServer : public utl::IPlugin
    {
        public:
            virtual ~INetworkServer() = default;

            // Server lifecycle
            virtual void init(const std::string &host, uint16_t port) = 0;
            virtual void start() = 0;
            virtual void stop() = 0;
            virtual void update() = 0;

            // Client management
            virtual void sendToClient(std::uint32_t sessionId, const std::vector<std::uint8_t> &data,
                                      bool reliable = false) = 0;
            virtual void sendToAllClients(const std::vector<std::uint8_t> &data, bool reliable = false) = 0;
            virtual void disconnectClient(std::uint32_t sessionId) = 0;

            // Server information
            [[nodiscard]] virtual std::size_t getClientCount() const = 0;
            [[nodiscard]] virtual std::vector<std::uint32_t> getConnectedSessions() const = 0;
            [[nodiscard]] virtual bool isRunning() const = 0;

            // Configuration
            virtual void setTickRate(std::uint16_t tickRate) = 0;
            virtual void setServerCapabilities(std::uint32_t caps) = 0;

        private:
    }; // class INetworkServer

} // namespace srv
