///
/// @file AsioServer.hpp
/// @brief Asio-based implementation of INetworkServer interface
/// @namespace srv
///

#pragma once

#include "Interfaces/INetworkServer.hpp"
#include "Interfaces/Protocol/HandlerPacket.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Interfaces/Protocol/Serializer.hpp"

#include <asio.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

namespace srv
{

    ///
    /// @brief Client session information
    ///
    struct ClientSession
    {
            std::uint32_t sessionId;
            asio::ip::udp::endpoint endpoint;
            std::chrono::steady_clock::time_point lastSeen;
            std::string playerName;
            std::uint32_t clientCaps;
            bool isConnected;
            std::uint32_t lastPingSent;
            std::uint32_t latency;

            ClientSession()
                : sessionId(0), lastSeen(std::chrono::steady_clock::now()), clientCaps(0), isConnected(false),
                  lastPingSent(0), latency(0)
            {
            }
    };

    ///
    /// @brief Queued packet for sending
    ///
    struct QueuedPacket
    {
            std::vector<std::uint8_t> data;
            asio::ip::udp::endpoint destination;
            bool reliable;

            QueuedPacket(const std::vector<std::uint8_t> &d, const asio::ip::udp::endpoint &dest, bool rel = false)
                : data(d), destination(dest), reliable(rel)
            {
            }
    };

    ///
    /// @class AsioServer
    /// @brief Asio UDP server implementation
    /// @namespace srv
    ///
    class AsioServer : public INetworkServer
    {
        private:
            // Network components
            std::unique_ptr<asio::io_context> m_ioContext;
            std::unique_ptr<asio::ip::udp::socket> m_socket;
            std::unique_ptr<std::thread> m_networkThread;

            // Server configuration
            std::string m_host;
            std::uint16_t m_port;
            std::uint16_t m_tickRate;
            std::uint32_t m_serverCaps;

            // Server state
            std::atomic<bool> m_running;
            std::atomic<bool> m_started;

            // Client management
            std::unordered_map<std::uint32_t, ClientSession> m_clients;
            std::unordered_map<std::string, std::uint32_t> m_endpointToSession;
            std::uint32_t m_nextSessionId;
            mutable std::mutex m_clientsMutex;

            // Packet handling
            std::unique_ptr<rnp::HandlerPacket> m_packetHandler;
            std::queue<QueuedPacket> m_sendQueue;
            std::mutex m_sendQueueMutex;
            std::mutex m_socketMutex;

            // Reception buffer
            std::array<std::uint8_t, MAX_LEN_RECV_BUFFER> m_recvBuffer;
            asio::ip::udp::endpoint m_senderEndpoint;

            // Timing
            std::chrono::steady_clock::time_point m_lastPingTime;
            std::chrono::milliseconds m_pingInterval;
            std::chrono::milliseconds m_clientTimeout;

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
            /// @brief Generate unique session ID
            /// @return New session ID
            ///
            std::uint32_t generateSessionId();

            ///
            /// @brief Get endpoint string representation
            /// @param endpoint UDP endpoint
            /// @return String representation
            ///
            std::string endpointToString(const asio::ip::udp::endpoint &endpoint) const;

            ///
            /// @brief Send packet immediately
            /// @param data Packet data
            /// @param destination Target endpoint
            ///
            void sendPacketImmediate(const std::vector<std::uint8_t> &data, const asio::ip::udp::endpoint &destination);

            ///
            /// @brief Handle CONNECT packet
            /// @param packet Connect packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleConnect(const rnp::PacketConnect &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle DISCONNECT packet
            /// @param packet Disconnect packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleDisconnect(const rnp::PacketDisconnect &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle PING packet
            /// @param packet Ping packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handlePing(const rnp::PacketPingPong &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle PONG packet
            /// @param packet Pong packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handlePong(const rnp::PacketPingPong &packet, const rnp::PacketContext &context);

            ///
            /// @brief Send PONG response
            /// @param nonce Ping nonce
            /// @param destination Target endpoint
            /// @param sessionId Client session ID
            ///
            void sendPong(std::uint32_t nonce, const asio::ip::udp::endpoint &destination, std::uint32_t sessionId);

            ///
            /// @brief Send CONNECT_ACCEPT response
            /// @param sessionId New session ID
            /// @param destination Target endpoint
            ///
            void sendConnectAccept(std::uint32_t sessionId, const asio::ip::udp::endpoint &destination);

            ///
            /// @brief Send ERROR packet
            /// @param errorCode Error code
            /// @param description Error description
            /// @param destination Target endpoint
            /// @param sessionId Session ID
            ///
            void sendError(rnp::ErrorCode errorCode, const std::string &description,
                           const asio::ip::udp::endpoint &destination, std::uint32_t sessionId);

            ///
            /// @brief Update client timeouts and send pings
            ///
            void updateClientManagement();

            ///
            /// @brief Process send queue
            ///
            void processSendQueue();

        public:
            ///
            /// @brief Constructor
            ///
            AsioServer();

            ///
            /// @brief Destructor
            ///
            ~AsioServer() override;

            [[nodiscard]] const std::string getName() const override { return "Network_Asio_Server"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_CLIENT; }

            // INetworkServer implementation
            void init(const std::string &host, std::uint16_t port) override;
            void start() override;
            void stop() override;
            void update() override;
            void sendToClient(std::uint32_t sessionId, const std::vector<std::uint8_t> &data,
                              bool reliable = false) override;
            void sendToAllClients(const std::vector<std::uint8_t> &data, bool reliable = false) override;
            void disconnectClient(std::uint32_t sessionId) override;

            [[nodiscard]] std::size_t getClientCount() const override;
            [[nodiscard]] std::vector<std::uint32_t> getConnectedSessions() const override;
            [[nodiscard]] bool isRunning() const override;

            void setTickRate(std::uint16_t tickRate) override;
            void setServerCapabilities(std::uint32_t caps) override;
    };

} // namespace srv
