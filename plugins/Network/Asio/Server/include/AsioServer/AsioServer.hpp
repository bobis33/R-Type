///
/// @file AsioServer.hpp
/// @brief This file contains the server network implementation for Asio
/// @namespace srv
///

#pragma once

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#define ASIO_STANDALONE
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
                    std::uint32_t sessionId;
                    std::uint32_t clientCaps;
            };

            AsioServer();
            ~AsioServer() override;

            AsioServer(const AsioServer &) = delete;
            AsioServer(AsioServer &&) = delete;
            AsioServer &operator=(const AsioServer &) = delete;
            AsioServer &operator=(AsioServer &&) = delete;

            void init(const std::string &host, uint16_t port) override;
            [[nodiscard]] const std::string getName() const override { return "Network_Asio_Server"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_SERVER; }

            void start() override;
            void stop() override;

            void sendConnectAccept(const asio::ip::udp::endpoint &client, std::uint32_t sessionId);
            void sendWorldState(const asio::ip::udp::endpoint &client, std::uint32_t serverTick,
                                const std::vector<rnp::EntityState> &entities);
            void sendWorldState(const asio::ip::udp::endpoint &client, const std::vector<uint8_t> &worldData);
            void sendEntityEvent(const asio::ip::udp::endpoint &client, std::uint32_t serverTick,
                                 const std::vector<rnp::EventRecord> &events);
            void sendEvents(const asio::ip::udp::endpoint &client, const std::vector<rnp::EventRecord> &events);
            void sendPong(const asio::ip::udp::endpoint &client, std::uint32_t nonce, std::uint32_t sendTimeMs);
            void sendPong(const asio::ip::udp::endpoint &client);
            void sendError(const asio::ip::udp::endpoint &client, rnp::ErrorCode errorCode,
                           const std::string &errorMessage);
            void sendError(const asio::ip::udp::endpoint &client, const std::string &errorMessage);
            void sendAck(const asio::ip::udp::endpoint &client, std::uint32_t cumulative, std::uint32_t ackBits);
            void broadcastToAll(const std::vector<uint8_t> &data);
            void broadcastEntityEvents(std::uint32_t serverTick, const std::vector<rnp::EventRecord> &events);
            void broadcastEvents(const std::vector<rnp::EventRecord> &events);

            void setPacketHandler(rnp::PacketType type, PacketHandler handler);
            void setTickRate(std::uint16_t tickRate) { m_tickRateHz = tickRate; }
            void setServerCapabilities(std::uint32_t caps) { m_serverCaps = caps; }

            const std::unordered_map<asio::ip::udp::endpoint, ClientInfo> &getClients() const { return m_clients; }

        private:
            void startReceive();
            void handleReceive(const asio::error_code &error, std::size_t bytesTransferred);
            void handleSend(const asio::error_code &error, std::size_t bytesTransferred);
            void processPacket(const asio::ip::udp::endpoint &sender, const std::vector<uint8_t> &data);
            void addClient(const asio::ip::udp::endpoint &endpoint, const std::string &playerName,
                           std::uint32_t clientCaps, std::uint32_t sessionId);
            void removeClient(const asio::ip::udp::endpoint &endpoint);
            std::uint16_t getPlayerId(const asio::ip::udp::endpoint &endpoint) const;
            std::uint32_t getSessionId(const asio::ip::udp::endpoint &endpoint) const;
            void handleReliablePacket(const asio::ip::udp::endpoint &sender, const rnp::PacketHeader &header);
            void processAck(const asio::ip::udp::endpoint &sender, const std::vector<uint8_t> &payload);
            void retransmitReliable();

            asio::io_context m_ioContext;
            asio::ip::udp::socket m_socket;
            asio::ip::udp::endpoint m_remoteEndpoint;
            std::array<uint8_t, rnp::MAX_PAYLOAD + 16> m_recvBuffer;

            std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;
            std::thread m_ioThread;
            std::unordered_map<asio::ip::udp::endpoint, ClientInfo> m_clients;
            std::unordered_map<rnp::PacketType, PacketHandler> m_packetHandlers;
            uint32_t m_sequenceNumber = 0;
            std::uint16_t m_nextPlayerId = 1;
            std::uint32_t m_nextSessionId = 1;
            std::uint16_t m_tickRateHz = 60;
            std::uint16_t m_mtuPayloadBytes = 508;
            std::uint32_t m_serverCaps = 0;
            std::unordered_map<std::uint32_t, std::vector<uint8_t>> m_pendingReliable;
            std::unordered_map<asio::ip::udp::endpoint, std::uint32_t> m_clientLastAck;
    }; // class AsioServer
} // namespace srv
