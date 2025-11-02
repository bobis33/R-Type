///
/// @file AsioClient.hpp
/// @brief Asio-based UDP client implementation for R-Type multiplayer game networking
/// @details This file provides a complete UDP client implementation using ASIO library.
///          It handles server connection, packet transmission/reception, lobby management,
///          and game state synchronization. The client implements the R-Type Network Protocol (RNP).
/// @namespace eng
/// @author R-Type Team
/// @date 2025
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
    /// @struct QueuedPacket
    /// @brief Represents a packet queued for asynchronous transmission to server
    /// @details Used in the send queue to buffer outgoing packets before transmission
    ///
    struct QueuedPacket
    {
            std::vector<std::uint8_t> data; ///< Serialized packet data
            bool reliable;                  ///< Whether packet requires reliable delivery

            ///
            /// @brief Constructor
            /// @param d Packet data buffer
            /// @param rel Reliability flag (default: false)
            ///
            explicit QueuedPacket(const std::vector<std::uint8_t> &d, const bool rel = false) : data(d), reliable(rel)
            {
            }
    };

    ///
    /// @struct ConnectionStats
    /// @brief Connection statistics tracking
    /// @details Tracks various network metrics for monitoring and debugging
    ///
    struct ConnectionStats
    {
            std::uint32_t packetsSent = 0;                        ///< Total number of packets sent
            std::uint32_t packetsReceived = 0;                    ///< Total number of packets received
            std::uint32_t bytesTransferred = 0;                   ///< Total bytes transferred (sent + received)
            std::uint32_t packetsLost = 0;                        ///< Estimated number of lost packets
            std::chrono::steady_clock::time_point connectionTime; ///< Timestamp when connection was established
    };

    ///
    /// @class AsioClient
    /// @brief High-performance UDP client implementation using ASIO library
    /// @details This class provides a complete multiplayer game client implementation with:
    ///          - Asynchronous UDP networking
    ///          - Server connection management
    ///          - Lobby system integration
    ///          - Packet routing and handling
    ///          - Event bus integration
    ///          - Thread-safe operations
    ///          - Latency measurement
    ///
    /// The client runs its network operations on a separate thread and communicates
    /// with the game engine via an event bus system.
    ///
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

            ///
            /// @brief Connect to game server
            /// @param host Server hostname or IP address
            /// @param port Server port number
            ///
            void connect(const std::string &host, std::uint16_t port) override;

            ///
            /// @brief Disconnect from server
            /// @details Sends DISCONNECT packet and closes connection
            ///
            void disconnect() override;

            ///
            /// @brief Update client state (called each frame)
            /// @details Processes send queue, checks timeouts, sends pings
            ///
            void update() override;

            [[nodiscard]] const std::string getName() const override { return "Network_Client"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_CLIENT; }

            ///
            /// @brief Check if client is connected to server
            /// @return True if connected
            ///
            [[nodiscard]] bool isConnected() const override;

            ///
            /// @brief Get current connection state
            /// @return Connection state enum
            ///
            [[nodiscard]] ConnectionState getConnectionState() const override;

            ///
            /// @brief Get assigned session ID
            /// @return Session ID (0 if not connected)
            ///
            [[nodiscard]] std::uint32_t getSessionId() const override;

            ///
            /// @brief Get server tick rate
            /// @return Server tick rate in Hz
            ///
            [[nodiscard]] std::uint16_t getServerTickRate() const override;

            ///
            /// @brief Get current latency to server
            /// @return Round-trip time in milliseconds
            ///
            [[nodiscard]] std::uint32_t getLatency() const override;

            ///
            /// @brief Set player name for connection
            /// @param playerName Display name
            ///
            void setPlayerName(const std::string &playerName) override;

            ///
            /// @brief Set client capability flags
            /// @param caps Capability flags (reserved)
            ///
            void setClientCapabilities(std::uint32_t caps) override;

            ///
            /// @brief Process events from event bus
            /// @details Handles events from game engine
            ///
            void processBusEvent();

            ///
            /// @brief Send custom packet to server
            /// @param data Serialized packet data
            /// @param reliable Whether packet requires reliable delivery (default: false)
            ///
            void sendToServer(const std::vector<std::uint8_t> &data, bool reliable = false) override;

            ///
            /// @brief Get connection statistics
            /// @return Current connection stats
            ///
            const ConnectionStats &getStats() const { return m_stats; }

            // Lobby management methods
            ///
            /// @brief Request list of available lobbies from server
            /// @details Sends LOBBY_LIST_REQUEST packet
            ///
            void requestLobbyList();

            ///
            /// @brief Create a new lobby on server
            /// @param lobbyCreate Lobby creation parameters (name, max players, game mode)
            /// @details Sends LOBBY_CREATE packet. Response handled via callback.
            ///
            void createLobby(rnp::PacketLobbyCreate lobbyCreate);

            ///
            /// @brief Join an existing lobby
            /// @param lobbyId Target lobby identifier
            /// @details Sends LOBBY_JOIN packet. Response handled via callback.
            ///
            void joinLobby(std::uint32_t lobbyId);

            ///
            /// @brief Leave current lobby
            /// @details Sends LOBBY_LEAVE packet and resets local lobby state
            ///
            void leaveLobby();

            ///
            /// @brief Request to start the game (host only)
            /// @param lobbyId Lobby ID to start
            /// @details Sends START_GAME_REQUEST packet. Only valid for lobby host.
            ///
            void requestStartGame(std::uint32_t lobbyId);

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
            /// @param callback Function(vector<LobbyInfo>) called when LOBBY_LIST_RESPONSE is received
            ///
            void setOnLobbyListReceived(std::function<void(const std::vector<rnp::LobbyInfo> &)> callback);

            ///
            /// @brief Set callback for lobby creation response
            /// @param callback Function(lobbyId, success, errorCode) called when LOBBY_CREATE_RESPONSE is received
            ///
            void setOnLobbyCreated(std::function<void(std::uint32_t, bool, rnp::ErrorCode)> callback);

            ///
            /// @brief Set callback for lobby join response
            /// @param callback Function(lobbyId, success, errorCode, lobbyInfo*) called when LOBBY_JOIN_RESPONSE is
            /// received
            ///
            void
            setOnLobbyJoined(std::function<void(std::uint32_t, bool, rnp::ErrorCode, const rnp::LobbyInfo *)> callback);

            ///
            /// @brief Set callback for lobby updates
            /// @param callback Function(LobbyInfo) called when LOBBY_UPDATE is received (player join/leave, status
            /// change)
            ///
            void setOnLobbyUpdated(std::function<void(const rnp::LobbyInfo &)> callback);

            ///
            /// @brief Set callback for game start notification
            /// @param callback Function(lobbyId, sessionId) called when GAME_START is received
            ///
            void setOnGameStart(std::function<void(std::uint32_t, std::uint32_t)> callback);

        private:
            // Network components
            std::unique_ptr<asio::io_context> m_ioContext;   ///< ASIO I/O context for async operations
            std::unique_ptr<asio::ip::udp::socket> m_socket; ///< UDP socket for network communication
            std::unique_ptr<std::thread> m_networkThread;    ///< Dedicated network thread

            // Server connection info
            std::string m_serverHost;                 ///< Server hostname or IP address
            std::uint16_t m_serverPort;               ///< Server port number
            asio::ip::udp::endpoint m_serverEndpoint; ///< Resolved server endpoint

            // Connection state
            std::atomic<ConnectionState> m_connectionState; ///< Current connection state (atomic)
            std::uint32_t m_sessionId;                      ///< Assigned session ID from server
            std::uint16_t m_serverTickRate;                 ///< Server tick rate in Hz
            std::uint32_t m_clientCaps;                     ///< Client capability flags

            // Client configuration
            std::string m_playerName; ///< Player display name

            // Network state
            std::atomic<bool> m_running; ///< Network thread running state (atomic)

            // Packet handling
            std::unique_ptr<rnp::HandlerPacket> m_packetHandler; ///< RNP packet handler instance
            std::queue<QueuedPacket> m_sendQueue;                ///< Queue of packets waiting to be sent
            std::mutex m_sendQueueMutex;                         ///< Mutex for send queue access

            // Reception buffer
            std::array<std::uint8_t, 1024> m_recvBuffer; ///< Buffer for receiving UDP packets
            asio::ip::udp::endpoint m_senderEndpoint;    ///< Endpoint of last packet sender

            // Ping/Latency management
            std::uint32_t m_lastPingNonce;                        ///< Nonce of last PING sent
            std::chrono::steady_clock::time_point m_lastPingTime; ///< Timestamp of last PING
            std::uint32_t m_latency;                              ///< Round-trip time in milliseconds
            std::chrono::milliseconds m_pingInterval;             ///< Interval between pings (5000ms)

            // Connection timeout
            std::chrono::steady_clock::time_point m_lastServerResponse; ///< Timestamp of last server packet
            std::chrono::milliseconds m_connectionTimeout;              ///< Timeout duration (15000ms)

            // Statistics
            ConnectionStats m_stats; ///< Connection statistics

            // Lobby state
            std::uint32_t m_currentLobbyId; ///< Current lobby ID, 0 if not in lobby

            // Lobby callbacks
            std::function<void(const std::vector<rnp::LobbyInfo> &)> m_onLobbyListReceived; ///< Lobby list callback
            std::function<void(std::uint32_t, bool, rnp::ErrorCode)> m_onLobbyCreated;      ///< Lobby created callback
            std::function<void(std::uint32_t, bool, rnp::ErrorCode, const rnp::LobbyInfo *)>
                m_onLobbyJoined;                                             ///< Lobby joined callback
            std::function<void(const rnp::LobbyInfo &)> m_onLobbyUpdated;    ///< Lobby updated callback
            std::function<void(std::uint32_t, std::uint32_t)> m_onGameStart; ///< Game start callback

            utl::EventBus &m_eventBus;                         ///< Event bus reference
            std::uint32_t m_componentId = utl::NETWORK_CLIENT; ///< Component ID for event bus

            ///
            /// @brief Setup packet handlers for all RNP packet types
            /// @details Registers callback functions for each packet type with the packet handler
            ///
            void setupPacketHandlers();

            ///
            /// @brief Start asynchronous receive operation
            /// @details Initiates async_receive_from on the UDP socket
            ///
            void startReceive();

            ///
            /// @brief Handle received packet data
            /// @param bytesReceived Number of bytes received from server
            /// @details Deserializes and dispatches packet to appropriate handler
            ///
            void handleReceive(std::size_t bytesReceived);

            ///
            /// @brief Main network thread loop
            /// @details Runs the ASIO io_context and handles network events
            ///
            void networkThreadLoop() const;

            ///
            /// @brief Send packet immediately via UDP socket
            /// @param data Serialized packet data
            /// @details Bypasses send queue for immediate transmission
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
            /// @brief Send CONNECT packet to initiate connection
            /// @details Sends player name and client capabilities
            ///
            void sendConnect();

            ///
            /// @brief Send DISCONNECT packet to terminate connection
            /// @details Includes disconnect reason code
            ///
            void sendDisconnect();

            ///
            /// @brief Send PING packet for latency measurement
            /// @details Generates and stores nonce for RTT calculation
            ///
            void sendPing();

            ///
            /// @brief Send PONG response to server PING
            /// @param nonce Ping nonce to echo back
            ///
            void sendPong(std::uint32_t nonce);

            ///
            /// @brief Process outgoing packet queue
            /// @details Sends all queued packets via UDP socket
            ///
            void processSendQueue();

            ///
            /// @brief Update connection management state
            /// @details Checks for timeouts and sends periodic pings
            ///
            void updateConnectionManagement();

            ///
            /// @brief Generate random ping nonce
            /// @return New cryptographically random nonce
            ///
            static std::uint32_t generatePingNonce();
    };

} // namespace eng
