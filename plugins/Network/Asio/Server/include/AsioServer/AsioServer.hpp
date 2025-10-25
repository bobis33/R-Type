///
/// @file AsioServer.hpp
/// @brief Asio-based implementation of INetworkServer interface
/// @namespace srv
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
            std::uint32_t currentLobbyId; // 0 if not in lobby

            ClientSession()
                : sessionId(0), lastSeen(std::chrono::steady_clock::now()), clientCaps(0), isConnected(false),
                  lastPingSent(0), latency(0), currentLobbyId(0)
            {
            }
    };

    ///
    /// @brief Lobby structure
    ///
    struct Lobby
    {
            std::uint32_t lobbyId;
            std::string lobbyName;
            std::uint32_t hostSessionId;
            std::vector<std::uint32_t> playerSessions;
            std::uint8_t maxPlayers;
            std::uint8_t gameMode;
            rnp::LobbyStatus status;
            std::chrono::steady_clock::time_point createdTime;

            Lobby()
                : lobbyId(0), lobbyName(""), hostSessionId(0), maxPlayers(0), gameMode(0),
                  status(rnp::LobbyStatus::WAITING), createdTime(std::chrono::steady_clock::now())
            {
            }
            Lobby(std::uint32_t id, const std::string &name, std::uint32_t host, std::uint8_t max, std::uint8_t mode)
                : lobbyId(id), lobbyName(name), hostSessionId(host), maxPlayers(max), gameMode(mode),
                  status(rnp::LobbyStatus::WAITING), createdTime(std::chrono::steady_clock::now())
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

            // Lobby management
            std::unordered_map<std::uint32_t, Lobby> m_lobbies;
            std::uint32_t m_nextLobbyId;
            mutable std::mutex m_lobbiesMutex;

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

            // EventBus
            std::uint32_t m_componentId;
            utl::EventBus &m_eventBus;

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
            /// @brief Generate unique session ID
            /// @return New session ID
            ///
            std::uint32_t generateSessionId() const;

            ///
            /// @brief Get endpoint string representation
            /// @param endpoint UDP endpoint
            /// @return String representation
            ///
            static std::string endpointToString(const asio::ip::udp::endpoint &endpoint);

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

            ///
            /// @brief Process EventBus events for network operations
            ///
            void processEventBusEvents();

            ///
            /// @brief Handle send to client event from EventBus
            /// @param event Send to client event
            ///
            void handleSendToClientEvent(const utl::Event &event);

            ///
            /// @brief Handle broadcast event from EventBus
            /// @param event Broadcast event
            ///
            void handleBroadcastEvent(const utl::Event &event);

            ///
            /// @brief Handle send entity event to clients from EventBus
            /// @param event Entity event to send to clients
            ///
            void handleSendEntityEventToClients(const utl::Event &event);

            ///
            /// @brief Handle entity event packet (including player inputs)
            /// @param events Entity event records
            /// @param context Packet context
            /// @return Handler result
            ///
            rnp::HandlerResult handleEntityEvent(const std::vector<rnp::EventRecord> &events,
                                                 const rnp::PacketContext &context);

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
            void sendLobbyList(std::uint32_t sessionId);

            ///
            /// @brief Create new lobby
            /// @param name Lobby name
            /// @param hostSession Host session ID
            /// @param maxPlayers Maximum players
            /// @param gameMode Game mode
            /// @return New lobby ID
            ///
            std::uint32_t createLobby(const std::string &name, std::uint32_t hostSession, std::uint8_t maxPlayers,
                                      std::uint8_t gameMode);

            ///
            /// @brief Join player to lobby
            /// @param lobbyId Target lobby ID
            /// @param sessionId Player session ID
            /// @return Success status
            ///
            bool joinLobby(std::uint32_t lobbyId, std::uint32_t sessionId);

            ///
            /// @brief Remove player from lobby
            /// @param sessionId Player session ID
            ///
            void leaveLobby(std::uint32_t sessionId);

            ///
            /// @brief Broadcast lobby update to all lobby members
            /// @param lobbyId Target lobby ID
            ///
            void broadcastLobbyUpdate(std::uint32_t lobbyId);

            ///
            /// @brief Convert Lobby to LobbyInfo for network transmission
            /// @param lobby Lobby structure
            /// @return LobbyInfo structure
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
            void sendLobbyJoinResponse(std::uint32_t sessionId, std::uint32_t lobbyId, bool success,
                                       rnp::ErrorCode errorCode, const rnp::LobbyInfo *lobbyInfo = nullptr);

            ///
            /// @brief Clean up empty lobbies
            ///
            void cleanupEmptyLobbies();
    };

} // namespace srv
