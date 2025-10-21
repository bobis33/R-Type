///
/// @file AsioClient.hpp
/// @brief Asio-based implementation of INetworkClient interface
/// @namespace eng
///

#pragma once

#include "Interfaces/INetworkClient.hpp"
#include "Interfaces/Protocol/HandlerPacket.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Utils/EventBus.hpp"

#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace eng
{

    ///
    /// @brief Queued packet for sending
    ///
    struct QueuedPacket
    {
            std::vector<std::uint8_t> data;
            bool reliable;

            QueuedPacket(const std::vector<std::uint8_t> &d, bool rel = false) : data(d), reliable(rel) {}
    };

    ///
    /// @brief Connection statistics
    ///
    struct ConnectionStats
    {
            std::uint32_t packetsSent = 0;
            std::uint32_t packetsReceived = 0;
            std::uint32_t bytesTransferred = 0;
            std::uint32_t packetsLost = 0;
            std::chrono::steady_clock::time_point connectionTime;
    };

    ///
    /// @class AsioClient
    /// @brief Asio UDP client implementation
    /// @namespace eng
    ///
    class AsioClient : public INetworkClient
    {
        public:
            ///
            /// @brief Constructor
            ///
            AsioClient();

            ///
            /// @brief Destructor
            ///
            ~AsioClient() override;

            // INetworkClient implementation
            void connect(const std::string &host, std::uint16_t port) override;
            void disconnect() override;

            void update() override;

            [[nodiscard]] const std::string getName() const override { return "Network_Client"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_CLIENT; }

            [[nodiscard]] bool isConnected() const override;
            [[nodiscard]] ConnectionState getConnectionState() const override;
            [[nodiscard]] std::uint32_t getSessionId() const override;
            [[nodiscard]] std::uint16_t getServerTickRate() const override;
            [[nodiscard]] std::uint32_t getLatency() const override;

            void setPlayerName(const std::string &playerName) override;
            void setClientCapabilities(std::uint32_t caps) override;

            void processBusEvent();

            ///
            /// @brief Send custom packet to server
            /// @param data Packet data
            /// @param reliable Whether packet should be reliable
            ///
            void sendToServer(const std::vector<std::uint8_t> &data, bool reliable = false) override;

            ///
            /// @brief Get connection statistics
            /// @return Current connection stats
            ///
            const ConnectionStats &getStats() const { return m_stats; }


                    private:
            // Network components
            std::unique_ptr<asio::io_context> m_ioContext;
            std::unique_ptr<asio::ip::udp::socket> m_socket;
            std::unique_ptr<std::thread> m_networkThread;

            // Server connection info
            std::string m_serverHost;
            std::uint16_t m_serverPort;
            asio::ip::udp::endpoint m_serverEndpoint;

            // Connection state
            std::atomic<ConnectionState> m_connectionState;
            std::uint32_t m_sessionId;
            std::uint16_t m_serverTickRate;
            std::uint32_t m_clientCaps;

            // Client configuration
            std::string m_playerName;

            // Network state
            std::atomic<bool> m_running;

            // Packet handling
            std::unique_ptr<rnp::HandlerPacket> m_packetHandler;
            std::queue<QueuedPacket> m_sendQueue;
            std::mutex m_sendQueueMutex;

            // Reception buffer
            std::array<std::uint8_t, 1024> m_recvBuffer;
            asio::ip::udp::endpoint m_senderEndpoint;

            // Ping/Latency management
            std::uint32_t m_lastPingNonce;
            std::chrono::steady_clock::time_point m_lastPingTime;
            std::uint32_t m_latency; // in milliseconds
            std::chrono::milliseconds m_pingInterval;

            // Connection timeout
            std::chrono::steady_clock::time_point m_lastServerResponse;
            std::chrono::milliseconds m_connectionTimeout;

            // Statistics
            ConnectionStats m_stats;

            utl::EventBus &m_eventBus;
            std::uint32_t m_componentId = 1;

            ///
            /// @brief Initialize packet handlers
            ///
            void setupPacketHandlers();

            ///
            /// @brief Start receiving packets asynchronously
            ///
            void startReceive();

            ///
            /// @brief Handle received packet
            /// @param bytesReceived Number of bytes received
            ///
            void handleReceive(std::size_t bytesReceived);

            ///
            /// @brief Network thread main loop
            ///
            void networkThreadLoop();

            ///
            /// @brief Send packet immediately
            /// @param data Packet data
            ///
            void sendPacketImmediate(const std::vector<std::uint8_t> &data);

            ///
            /// @brief Handle CONNECT_ACCEPT packet
            /// @param packet Connect accept packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleConnectAccept(const rnp::PacketConnectAccept &packet,
                                                   const rnp::PacketContext &context);

            ///
            /// @brief Handle DISCONNECT packet
            /// @param packet Disconnect packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleDisconnect(const rnp::PacketDisconnect &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle PONG packet
            /// @param packet Pong packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handlePong(const rnp::PacketPingPong &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle PING packet
            /// @param packet Ping packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handlePing(const rnp::PacketPingPong &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle ERROR packet
            /// @param packet Error packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleError(const rnp::PacketError &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle WORLD_STATE packet
            /// @param packet World state packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleWorldState(const rnp::PacketWorldState &packet, const rnp::PacketContext &context);

            ///
            /// @brief Send CONNECT packet
            ///
            void sendConnect();

            ///
            /// @brief Send DISCONNECT packet
            ///
            void sendDisconnect();

            ///
            /// @brief Send PING packet
            ///
            void sendPing();

            ///
            /// @brief Send PONG response
            /// @param nonce Ping nonce to echo back
            ///
            void sendPong(std::uint32_t nonce);

            ///
            /// @brief Process send queue
            ///
            void processSendQueue();

            ///
            /// @brief Update connection management (timeouts, pings)
            ///
            void updateConnectionManagement();

            ///
            /// @brief Generate ping nonce
            /// @return New ping nonce
            ///
            std::uint32_t generatePingNonce();
    };

} // namespace eng
