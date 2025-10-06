///
/// @file AsioClient.hpp
/// @brief This file contains the client network implementation for Asio
/// @namespace eng
///

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "asio.hpp"

#include "Interfaces/INetworkClient.hpp"
#include "Interfaces/Protocol/Protocol.hpp"

namespace eng
{

    ///
    /// @class AsioClient
    /// @brief Network implementation with asio for client
    /// @namespace eng
    ///
    class AsioClient final : public INetworkClient
    {
        public:
            using PacketHandler = std::function<void(const rnp::PacketHeader &, const std::vector<uint8_t> &)>;

            AsioClient();
            ~AsioClient() override = default;

            AsioClient(const AsioClient &) = delete;
            AsioClient(AsioClient &&) = delete;
            AsioClient &operator=(const AsioClient &) = delete;
            AsioClient &operator=(AsioClient &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Network_Asio_Client"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_CLIENT; }

            void connect(const std::string &host, uint16_t port) override;
            void disconnect() override;

            void sendConnect(const std::string &playerName);
            void sendConnectWithCaps(const std::string &playerName, std::uint32_t clientCaps);
            void sendDisconnect();
            void sendDisconnect(rnp::DisconnectReason reason);
            void sendPlayerInput(uint8_t direction, uint8_t shooting);
            void sendPlayerInputAsEvent(std::uint16_t playerId, uint8_t direction, uint8_t shooting,
                                       uint32_t clientTimeMs);
            void sendPing();
            void sendPing(std::uint32_t nonce, std::uint32_t sendTimeMs);
            void sendAck(std::uint32_t cumulative, std::uint32_t ackBits);

            void setPacketHandler(rnp::PacketType type, PacketHandler handler);

            // Gestion dédiée aux événements
            void setEventsHandler(std::function<void(const std::vector<rnp::EventRecord> &)> handler);
            
            // Getters
            std::uint32_t getSessionId() const { return m_sessionId; }
            std::uint16_t getServerTickRate() const { return m_serverTickRate; }

        private:
            void startReceive();
            void handleReceive(const asio::error_code &error, std::size_t bytesTransferred);
            void handleSend(const asio::error_code &error, std::size_t bytesTransferred);
            void processPacket(const std::vector<uint8_t> &data);
            void processEvents(const std::vector<uint8_t> &payload);
            void handleConnectAccept(const std::vector<uint8_t> &payload);
            void handleReliablePacket(const rnp::PacketHeader &header);
            void processAck(const std::vector<uint8_t> &payload);
            void processWorldState(const std::vector<uint8_t> &payload);
            void processEntityEvent(const std::vector<uint8_t> &payload);
            void retransmitReliable();

            asio::io_context m_ioContext;
            asio::ip::udp::socket m_socket;
            asio::ip::udp::endpoint m_serverEndpoint;
            std::array<uint8_t, rnp::MAX_PAYLOAD + 16> m_recvBuffer;

            std::optional<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;
            std::thread m_ioThread;
            std::unordered_map<rnp::PacketType, PacketHandler> m_packetHandlers;
            std::function<void(const std::vector<rnp::EventRecord> &)> m_eventsHandler;
            uint32_t m_sequenceNumber = 0;
            bool m_connected = false;
            std::uint32_t m_sessionId = 0;
            std::uint16_t m_serverTickRate = 0;
            std::uint16_t m_serverMtu = 0;
            std::uint32_t m_serverCaps = 0;
            std::uint32_t m_clientCaps = 0;
            std::unordered_map<std::uint32_t, std::vector<uint8_t>> m_pendingReliable;
            std::uint32_t m_lastAckSent = 0;
    }; // class AsioClient
} // namespace eng
