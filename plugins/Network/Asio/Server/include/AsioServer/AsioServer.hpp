///
/// @file AsioServer.hpp
/// @brief This file contains the server network implementation for Asio
/// @namespace srv
///

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "asio.hpp"

#include "Interfaces/INetworkServer.hpp"
#include "Interfaces/Protocol/Protocol.hpp"

namespace srv
{

    ///
    /// @class AsioServer
    /// @brief Network implementation with asio for server
    /// @namespace srv
    ///
    class AsioServer final : public INetworkServer
    {
        public:
            using PacketHandler = std::function<void(const asio::ip::udp::endpoint &, const rnp::PacketHeader &,
                                                     const std::vector<uint8_t> &)>;
            using ClientInfo = struct
            {
                    asio::ip::udp::endpoint endpoint;
                    std::string playerName;
                    uint32_t lastSequence;
                    bool connected;
                    std::uint16_t playerId;
            };

            AsioServer(uint16_t port, const std::string &address);
            ~AsioServer() override { stop(); }

            AsioServer(const AsioServer &) = delete;
            AsioServer(AsioServer &&) = delete;
            AsioServer &operator=(const AsioServer &) = delete;
            AsioServer &operator=(AsioServer &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Network_Asio_Server"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_SERVER; }

            void init(uint16_t port, const std::string &address) override;
            void start() override;
            void stop() override;

            void sendWorldState(const asio::ip::udp::endpoint &client, const std::vector<uint8_t> &worldData);
            void sendEvents(const asio::ip::udp::endpoint &client, const std::vector<rnp::EventRecord> &events);
            void sendPong(const asio::ip::udp::endpoint &client);
            void sendError(const asio::ip::udp::endpoint &client, const std::string &errorMessage);
            void broadcastToAll(const std::vector<uint8_t> &data);
            void broadcastEvents(const std::vector<rnp::EventRecord> &events);

            void setPacketHandler(rnp::PacketType type, PacketHandler handler);

            const std::unordered_map<asio::ip::udp::endpoint, ClientInfo> &getClients() const { return m_clients; }

        private:
            void startReceive();
            void handleReceive(const asio::error_code &error, std::size_t bytesTransferred);
            void handleSend(const asio::error_code &error, std::size_t bytesTransferred);
            void processPacket(const asio::ip::udp::endpoint &sender, const std::vector<uint8_t> &data);
            void addClient(const asio::ip::udp::endpoint &endpoint, const std::string &playerName);
            void removeClient(const asio::ip::udp::endpoint &endpoint);
            std::uint16_t getPlayerId(const asio::ip::udp::endpoint &endpoint) const;

            asio::io_context m_ioContext;
            asio::ip::udp::socket m_socket;
            asio::ip::udp::endpoint m_remoteEndpoint;
            std::array<uint8_t, rnp::MAX_PAYLOAD + sizeof(rnp::PacketHeader)> m_recvBuffer;

            std::optional<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;
            std::thread m_ioThread;
            std::unordered_map<asio::ip::udp::endpoint, ClientInfo> m_clients;
            std::unordered_map<rnp::PacketType, PacketHandler> m_packetHandlers;
            uint32_t m_sequenceNumber = 0;
            std::uint16_t m_nextPlayerId = 1;
    }; // class AsioServer
} // namespace srv
