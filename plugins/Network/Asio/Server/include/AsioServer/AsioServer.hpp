///
/// @file AsioServer.hpp
/// @brief Asio-based UDP server implementation for R-Type multiplayer game networking
/// @details This file provides a complete UDP server implementation using ASIO library.
///          It handles client connections, packet routing, lobby management, and game state synchronization.
///          The server implements the R-Type Network Protocol (RNP) specification.
/// @namespace srv
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "Interfaces/INetworkServer.hpp"
#include "Interfaces/Protocol/HandlerPacket.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Utils/Event.hpp"
#include "Utils/EventBus.hpp"

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
    /// @struct ClientSession
    /// @brief Represents an active client connection session
    /// @details Stores all relevant information about a connected client including
    ///          network endpoint, authentication state, latency metrics, and lobby membership.
    ///
    struct ClientSession
    {
            std::uint32_t sessionId;                        ///< Unique session identifier assigned by server
            asio::ip::udp::endpoint endpoint;               ///< Client's network endpoint (IP:port)
            std::chrono::steady_clock::time_point lastSeen; ///< Timestamp of last received packet
            std::string playerName;                         ///< Player's display name
            std::uint32_t clientCaps;                       ///< Client capability flags (reserved)
            bool isConnected;                               ///< Connection state flag
            std::uint32_t lastPingSent;                     ///< Nonce of last PING packet sent
            std::uint32_t latency;                          ///< Round-trip time in milliseconds
            std::uint32_t currentLobbyId;                   ///< ID of joined lobby, 0 if not in lobby

            ///
            /// @brief Default constructor
            /// @details Initializes a client session with default values
            ///
            ClientSession()
                : sessionId(0), lastSeen(std::chrono::steady_clock::now()), clientCaps(0), isConnected(false),
                  lastPingSent(0), latency(0), currentLobbyId(0)
            {
            }
    };

    ///
    /// @enum LobbyStatus
    /// @brief Status codes for lobby operations
    /// @details Used to indicate the result of lobby-related operations
    ///
    enum class LobbyStatus : std::uint8_t
    {
        SUCCESS = 0,         ///< Operation completed successfully
        WAITING = 1,         ///< Lobby is waiting for players
        IN_GAME = 2,         ///< Game in progress
        NOT_FOUND = 3,       ///< Requested lobby does not exist
        FULL = 4,            ///< Lobby has reached maximum capacity
        ALREADY_IN_LOBBY = 5 ///< Client is already in a lobby
    };

    ///
    /// @struct Lobby
    /// @brief Represents a game lobby (room) where players gather before starting a game
    /// @details Contains all lobby metadata including players, host, game mode, and status.
    ///          Lobbies are managed by the server and synchronized across all members.
    ///
    struct Lobby
    {
            std::uint32_t lobbyId;                             ///< Unique lobby identifier
            std::string lobbyName;                             ///< Human-readable lobby name
            std::uint32_t hostSessionId;                       ///< Session ID of the lobby host
            std::vector<std::uint32_t> playerSessions;         ///< List of player session IDs in this lobby
            std::uint8_t maxPlayers;                           ///< Maximum number of players allowed (1-8)
            std::uint8_t gameMode;                             ///< Game mode identifier
            rnp::LobbyStatus status;                           ///< Current lobby status
            std::chrono::steady_clock::time_point createdTime; ///< Timestamp when lobby was created

            ///
            /// @brief Default constructor
            /// @details Creates an empty lobby with default values
            ///
            Lobby()
                : lobbyId(0), lobbyName(""), hostSessionId(0), maxPlayers(0), gameMode(0),
                  status(rnp::LobbyStatus::WAITING), createdTime(std::chrono::steady_clock::now())
            {
            }

            ///
            /// @brief Parameterized constructor
            /// @param id Unique lobby identifier
            /// @param name Lobby display name
            /// @param host Session ID of the host player
            /// @param max Maximum number of players (1-8)
            /// @param mode Game mode identifier
            ///
            Lobby(std::uint32_t id, const std::string &name, std::uint32_t host, std::uint8_t max, std::uint8_t mode)
                : lobbyId(id), lobbyName(name), hostSessionId(host), maxPlayers(max), gameMode(mode),
                  status(rnp::LobbyStatus::WAITING), createdTime(std::chrono::steady_clock::now())
            {
            }
    };

    ///
    /// @struct QueuedPacket
    /// @brief Represents a packet queued for asynchronous transmission
    /// @details Used in the send queue to buffer outgoing packets before transmission
    ///
    struct QueuedPacket
    {
            std::vector<std::uint8_t> data;      ///< Serialized packet data
            asio::ip::udp::endpoint destination; ///< Target endpoint
            bool reliable;                       ///< Whether packet requires reliable delivery

            ///
            /// @brief Constructor
            /// @param d Packet data buffer
            /// @param dest Destination endpoint
            /// @param rel Reliability flag (default: false)
            ///
            QueuedPacket(const std::vector<std::uint8_t> &d, const asio::ip::udp::endpoint &dest, bool rel = false)
                : data(d), destination(dest), reliable(rel)
            {
            }
    };

    ///
    /// @class AsioServer
    /// @brief High-performance UDP server implementation using ASIO library
    /// @details This class provides a complete multiplayer game server implementation with:
    ///          - Asynchronous UDP networking
    ///          - Client session management
    ///          - Lobby system for matchmaking
    ///          - Packet routing and handling
    ///          - Event bus integration
    ///          - Thread-safe operations
    ///
    /// The server runs its network operations on a separate thread and communicates
    /// with the game engine via an event bus system.
    ///
    /// @namespace srv
    ///
    class AsioServer final : public INetworkServer
    {

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

        private:
            // Network components
            std::unique_ptr<asio::io_context> m_ioContext;   ///< ASIO I/O context for async operations
            std::unique_ptr<asio::ip::udp::socket> m_socket; ///< UDP socket for network communication
            std::unique_ptr<std::thread> m_networkThread;    ///< Dedicated network thread

            // Server configuration
            std::string m_host;         ///< Server bind address (e.g., "0.0.0.0")
            std::uint16_t m_port;       ///< Server listening port (default: 4567)
            std::uint16_t m_tickRate;   ///< Server tick rate in Hz
            std::uint32_t m_serverCaps; ///< Server capability flags

            // Server state
            std::atomic<bool> m_running; ///< Server running state (atomic for thread safety)
            std::atomic<bool> m_started; ///< Server started state

            // Client management
            std::unordered_map<std::uint32_t, ClientSession> m_clients; ///< Map of session ID to client session
            std::unordered_map<std::string, std::uint32_t>
                m_endpointToSession;           ///< Map of endpoint string to session ID
            std::uint32_t m_nextSessionId;     ///< Next available session ID
            mutable std::mutex m_clientsMutex; ///< Mutex for thread-safe client access

            // Lobby management
            std::unordered_map<std::uint32_t, Lobby> m_lobbies; ///< Map of lobby ID to lobby data
            std::uint32_t m_nextLobbyId;                        ///< Next available lobby ID
            mutable std::mutex m_lobbiesMutex;                  ///< Mutex for thread-safe lobby access

            // Packet handling
            std::unique_ptr<rnp::HandlerPacket> m_packetHandler; ///< RNP packet handler instance
            std::queue<QueuedPacket> m_sendQueue;                ///< Queue of packets waiting to be sent
            std::mutex m_sendQueueMutex;                         ///< Mutex for send queue access
            std::mutex m_socketMutex;                            ///< Mutex for socket operations

            // Reception buffer
            std::array<std::uint8_t, MAX_LEN_RECV_BUFFER> m_recvBuffer; ///< Buffer for receiving UDP packets
            asio::ip::udp::endpoint m_senderEndpoint;                   ///< Endpoint of last packet sender

            // Timing
            std::chrono::steady_clock::time_point m_lastPingTime; ///< Timestamp of last ping sweep
            std::chrono::milliseconds m_pingInterval;             ///< Interval between ping sweeps (5000ms)
            std::chrono::milliseconds m_clientTimeout;            ///< Client timeout duration (15000ms)

            // EventBus
            std::uint32_t m_componentId; ///< Component ID for event bus registration
            utl::EventBus &m_eventBus;

            ///
            /// @brief Initialize packet handlers
            ///
            ///
            /// @brief Setup packet handlers for all RNP packet types
            /// @details Registers callback functions for each packet type with the packet handler
            ///
            void setupPacketHandlers();

            ///
            /// @brief Start receiving packets asynchronously
            ///
            ///
            /// @brief Start asynchronous receive operation
            /// @details Initiates async_receive_from on the UDP socket
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
            ///
            /// @brief Main network thread loop
            /// @details Runs the ASIO io_context and handles network events
            ///
            void networkThreadLoop() const;

            ///
            /// @brief Generate unique session ID
            /// @return New session ID
            ///
            ///
            /// @brief Generate a unique session ID
            /// @return New session ID (cryptographically random)
            ///
            std::uint32_t generateSessionId() const;

            ///
            /// @brief Get endpoint string representation
            /// @param endpoint UDP endpoint
            /// @return String representation
            ///
            ///
            /// @brief Convert endpoint to string representation
            /// @param endpoint UDP endpoint
            /// @return String in format "IP:PORT"
            ///
            std::string endpointToString(const asio::ip::udp::endpoint &endpoint);

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
            ///
            /// @brief Update client management (timeouts, pings)
            /// @details Checks for timed-out clients and sends periodic ping packets
            ///
            void updateClientManagement();

            ///
            /// @brief Process send queue
            ///
            ///
            /// @brief Process outgoing packet queue
            /// @details Sends all queued packets via UDP socket
            ///
            void processSendQueue();

            ///
            /// @brief Process EventBus events for network operations
            ///
            ///
            /// @brief Process events from the event bus
            /// @details Handles events from game engine (sends, broadcasts, etc.)
            ///
            void processEventBusEvents();

            ///
            /// @brief Handle send to client event from EventBus
            /// @param event Send to client event
            ///
            ///
            /// @brief Handle "send to client" event from event bus
            /// @param event Event containing target session and packet data
            ///
            void handleSendToClientEvent(const utl::Event &event);

            ///
            /// @brief Handle broadcast event from EventBus
            /// @param event Broadcast event
            ///
            ///
            /// @brief Handle broadcast event from event bus
            /// @param event Event containing packet data to broadcast
            ///
            void handleBroadcastEvent(const utl::Event &event);

            ///
            /// @brief Handle send entity event to clients from EventBus
            /// @param event Entity event to send to clients
            ///
            ///
            /// @brief Handle entity event broadcast to clients
            /// @param event Event containing entity event data
            ///
            void handleSendEntityEventToClients(const utl::Event &event);

            ///
            /// @brief Handle entity event packet (including player inputs)
            /// @param events Entity event records
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleEntityEvent(const std::vector<rnp::EventRecord> &events,
                                                 const rnp::PacketContext &context) const;

            ///
            /// @brief Handle LOBBY_LIST_REQUEST packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyListRequest(const rnp::PacketContext &context);

            ///
            /// @brief Handle LOBBY_CREATE packet
            /// @param packet Lobby create packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyCreate(const rnp::PacketLobbyCreate &packet,
                                                 const rnp::PacketContext &context);

            ///
            /// @brief Handle LOBBY_JOIN packet
            /// @param packet Lobby join packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyJoin(const rnp::PacketLobbyJoin &packet, const rnp::PacketContext &context);

            ///
            /// @brief Handle LOBBY_LEAVE packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleLobbyLeave(const rnp::PacketContext &context);

            ///
            /// @brief Send lobby list to client
            /// @param sessionId Target client session ID
            ///
            ///
            /// @brief Send lobby list to requesting client
            /// @param sessionId Requesting client's session ID
            ///
            void sendLobbyList(std::uint32_t sessionId);

            ///
            /// @brief Create new lobby
            /// @param name Lobby name
            /// @param hostSession Host session ID
            /// @param maxPlayers Maximum players
            /// @param gameMode Game mode
            /// @return New lobby ID
            ///
            ///
            /// @brief Create a new lobby
            /// @param lobbyName Display name for the lobby
            /// @param hostSessionId Session ID of the lobby host
            /// @param maxPlayers Maximum number of players (1-8)
            /// @param gameMode Game mode identifier
            /// @return New lobby ID
            ///
            std::uint32_t createLobby(const std::string &lobbyName, std::uint32_t hostSessionId,
                                      std::uint8_t maxPlayers, std::uint8_t gameMode);

            ///
            /// @brief Join player to lobby
            /// @param lobbyId Target lobby ID
            /// @param sessionId Player session ID
            /// @return Success status
            ///
            ///
            /// @brief Join an existing lobby
            /// @param lobbyId Target lobby ID
            /// @param sessionId Joining client's session ID
            /// @return Status code indicating success or failure reason
            ///
            LobbyStatus joinLobby(std::uint32_t lobbyId, std::uint32_t sessionId);

            ///
            /// @brief Remove player from lobby
            /// @param sessionId Player session ID
            ///
            ///
            /// @brief Remove a client from their current lobby
            /// @param sessionId Client's session ID
            ///
            void leaveLobby(std::uint32_t sessionId);

            ///
            /// @brief Broadcast lobby update to all lobby members
            /// @param lobbyId Target lobby ID
            ///
            ///
            /// @brief Broadcast lobby update to all lobby members
            /// @param lobbyId Lobby ID to broadcast update for
            ///
            void broadcastLobbyUpdate(std::uint32_t lobbyId);

            ///
            /// @brief Handle START_GAME_REQUEST packet
            /// @param packet Start game request packet
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleStartGameRequest(const rnp::PacketStartGameRequest &packet,
                                                      const rnp::PacketContext &context);

            ///
            /// @brief Broadcast game start to all players in lobby
            /// @param lobbyId Target lobby ID
            ///
            ///
            /// @brief Broadcast game start notification to all lobby members
            /// @param lobbyId Lobby ID that is starting
            ///
            ///
            /// @brief Broadcast game over notification to lobby members
            /// @param lobbyId Lobby ID that finished
            ///
            void broadcastGameOverToLobby(std::uint32_t lobbyId);

            ///
            /// @brief Broadcast game start notification to all lobby members
            /// @param lobbyId Lobby ID that is starting
            ///
            void broadcastGameStart(std::uint32_t lobbyId);

            ///
            /// @brief Convert Lobby to LobbyInfo for network transmission
            /// @param lobby Lobby structure
            /// @return LobbyInfo structure
            ///
            ///
            /// @brief Convert internal Lobby struct to RNP LobbyInfo
            /// @param lobby Internal lobby structure
            /// @return RNP protocol lobby info structure
            ///
            rnp::LobbyInfo lobbyToLobbyInfo(const Lobby &lobby);

            ///
            /// @brief Send lobby create response
            /// @param sessionId Target client session ID
            /// @param lobbyId Created lobby ID (0 if failed)
            /// @param success Success status
            /// @param errorCode Error code if failed
            ///
            void sendLobbyCreateResponse(std::uint32_t sessionId, std::uint32_t lobbyId, bool success,
                                         rnp::ErrorCode errorCode = rnp::ErrorCode::INTERNAL_ERROR);

            ///
            /// @brief Send lobby join response
            /// @param sessionId Target client session ID
            /// @param lobbyId Target lobby ID
            /// @param success Success status
            /// @param errorCode Error code if failed
            /// @param lobbyInfo Lobby information if successful
            ///
            ///
            /// @brief Send lobby join response to client
            /// @param sessionId Requesting client's session ID
            /// @param lobbyId Target lobby ID
            /// @param success Whether join succeeded
            /// @param errorCode Error code if failed
            /// @param lobbyInfo Current lobby info (if success)
            ///
            void sendLobbyJoinResponse(std::uint32_t sessionId, std::uint32_t lobbyId, bool success,
                                       rnp::ErrorCode errorCode, const rnp::LobbyInfo *lobbyInfo = nullptr);

            ///
            /// @brief Handle GAME_OVER event from game server
            /// @param event Game over event
            ///
            ///
            /// @brief Handle game over event from game engine
            /// @param event Event containing game over data
            ///
            void handleGameOverEvent(const utl::Event &event);

            ///
            /// @brief Broadcast GAME_OVER packet to all players in a lobby
            /// @param lobbyId Target lobby ID
            /// @param gameOverData Game over packet data
            ///
            void broadcastGameOverToLobby(std::uint32_t lobbyId, const std::vector<std::uint8_t> &gameOverData);

            ///
            /// @brief Clean up empty lobbies
            ///
            ///
            /// @brief Remove empty lobbies from server
            /// @details Called periodically to clean up abandoned lobbies
            ///
            void cleanupEmptyLobbies();
    };

} // namespace srv
