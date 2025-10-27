///
/// @file AsioClient.hpp
/// @brief Asio-based implementation of INetworkClient interface
/// @namespace eng
///

#pragma once

#include "Interfaces/INetworkClient.hpp"
#include "Interfaces/Protocol/HandlerPacket.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Utils/Event.hpp"
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

            explicit QueuedPacket(const std::vector<std::uint8_t> &d, const bool rel = false) : data(d), reliable(rel)
            {
            }
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
    class AsioClient final : public INetworkClient
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

            // Lobby management methods
            ///
            /// @brief Request lobby list from server
            ///
            void requestLobbyList();

            ///
            /// @brief Create a new lobby
            /// @param name Lobby name
            /// @param maxPlayers Maximum number of players
            /// @param gameMode Game mode identifier
            ///
            void createLobby(rnp::PacketLobbyCreate lobbyCreate);

            ///
            /// @brief Join an existing lobby
            /// @param lobbyId ID of the lobby to join
            ///
            void joinLobby(std::uint32_t lobbyId);

            ///
            /// @brief Leave current lobby
            ///
            void leaveLobby();

            ///
            /// @brief Get current lobby ID
            /// @return Current lobby ID (0 if not in lobby)
            ///
            std::uint32_t getCurrentLobbyId() const { return m_currentLobbyId; }

            ///
            /// @brief Check if player is in a lobby
            /// @return True if in lobby
            ///
            bool isInLobby() const { return m_currentLobbyId != 0; }

            // Callback setters for lobby events
            ///
            /// @brief Set callback for lobby list received
            /// @param callback Function to call when lobby list is received
            ///
            void setOnLobbyListReceived(std::function<void(const std::vector<rnp::LobbyInfo> &)> callback);

            ///
            /// @brief Set callback for lobby created
            /// @param callback Function to call when lobby creation response is received
            ///
            void setOnLobbyCreated(std::function<void(std::uint32_t, bool, rnp::ErrorCode)> callback);

            ///
            /// @brief Set callback for lobby joined
            /// @param callback Function to call when lobby join response is received
            ///
            void
            setOnLobbyJoined(std::function<void(std::uint32_t, bool, rnp::ErrorCode, const rnp::LobbyInfo *)> callback);

            ///
            /// @brief Set callback for lobby updated
            /// @param callback Function to call when lobby is updated
            ///
            void setOnLobbyUpdated(std::function<void(const rnp::LobbyInfo &)> callback);

            ///
            /// @brief Set callback for game start
            /// @param callback Function to call when game starts
            ///
            void setOnGameStart(std::function<void(std::uint32_t, std::uint32_t)> callback);

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

            // Lobby state
            std::uint32_t m_currentLobbyId;

            // Lobby callbacks
            std::function<void(const std::vector<rnp::LobbyInfo> &)> m_onLobbyListReceived;
            std::function<void(std::uint32_t, bool, rnp::ErrorCode)> m_onLobbyCreated;
            std::function<void(std::uint32_t, bool, rnp::ErrorCode, const rnp::LobbyInfo *)> m_onLobbyJoined;
            std::function<void(const rnp::LobbyInfo &)> m_onLobbyUpdated;
            std::function<void(std::uint32_t, std::uint32_t)> m_onGameStart;

            utl::EventBus &m_eventBus;
            std::uint32_t m_componentId = utl::NETWORK_CLIENT;

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
            void networkThreadLoop() const;

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
            static rnp::HandlerResult handleError(const rnp::PacketError &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle WORLD_STATE packet
            /// @param packet World state packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleWorldState(const rnp::PacketWorldState &packet,
                                                const rnp::PacketContext &context) const;

            ///
            /// @brief Handle ENTITY_EVENT packet
            /// @param events Vector of event records
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleEntityEvent(const std::vector<rnp::EventRecord> &events,
                                                 const rnp::PacketContext &context) const;

            ///
            /// @brief Handle LOBBY_LIST_RESPONSE packet
            /// @param packet Lobby list response packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyListResponse(const rnp::PacketLobbyListResponse &packet,
                                                       const rnp::PacketContext &context) const;

            ///
            /// @brief Handle LOBBY_CREATE_RESPONSE packet
            /// @param packet Lobby create response packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyCreateResponse(const rnp::PacketLobbyCreateResponse &packet,
                                                         const rnp::PacketContext &context);

            ///
            /// @brief Handle LOBBY_JOIN_RESPONSE packet
            /// @param packet Lobby join response packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyJoinResponse(const rnp::PacketLobbyJoinResponse &packet,
                                                       const rnp::PacketContext &context);

            ///
            /// @brief Handle LOBBY_UPDATE packet
            /// @param packet Lobby update packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyUpdate(const rnp::PacketLobbyUpdate &packet,
                                                 const rnp::PacketContext &context) const;

            ///
            /// @brief Handle GAME_START packet
            /// @param packet Game start packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleGameStart(const rnp::PacketGameStart &packet,
                                               const rnp::PacketContext &context) const;

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
            static std::uint32_t generatePingNonce();
    };

} // namespace eng
