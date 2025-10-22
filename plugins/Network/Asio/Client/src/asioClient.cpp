///
/// @file AsioClient.cpp
/// @brief Asio-based implementation of INetworkClient interface
/// @namespace eng
///

#include "AsioClient/AsioClient.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "Utils/Event.hpp"
#include "Utils/Logger.hpp"
#include <cstring>

#include <chrono>
#include <iostream>
#include <random>

namespace eng
{

    AsioClient::AsioClient()
        : m_ioContext(std::make_unique<asio::io_context>()),
          m_socket(std::make_unique<asio::ip::udp::socket>(*m_ioContext)), m_serverPort(0),
          m_connectionState(ConnectionState::DISCONNECTED), m_sessionId(0), m_serverTickRate(60), m_clientCaps(0),
          m_running(false), m_packetHandler(std::make_unique<rnp::HandlerPacket>()), m_lastPingNonce(0), m_latency(0),
          m_pingInterval(std::chrono::seconds(5)), m_connectionTimeout(std::chrono::seconds(30)),
          m_eventBus(utl::EventBus::getInstance()), m_componentId(utl::NETWORK_CLIENT)
    {
        m_stats.connectionTime = std::chrono::steady_clock::now();
        setupPacketHandlers();
        utl::Logger::log("AsioClient: Initialized", utl::LogLevel::INFO);

        m_eventBus.registerComponent(m_componentId, "Asio Client");
        m_eventBus.subscribe(m_componentId, utl::EventType::SEND_PLAYER_INPUT);
        m_eventBus.subscribe(m_componentId, utl::EventType::REQUEST_CONNECT);
        m_eventBus.subscribe(m_componentId, utl::EventType::REQUEST_DISCONNECT);
        m_eventBus.subscribe(m_componentId, utl::EventType::SEND_ENTITY_EVENT);
    }

    AsioClient::~AsioClient()
    {
        AsioClient::disconnect();
        utl::Logger::log("AsioClient: Destroyed", utl::LogLevel::INFO);
    }

    void AsioClient::connect(const std::string &host, std::uint16_t port)
    {
        if (m_connectionState.load() != ConnectionState::DISCONNECTED)
        {
            utl::Logger::log("AsioClient: Already connected or connecting", utl::LogLevel::WARNING);
            return;
        }

        m_serverHost = host;
        m_serverPort = port;
        m_connectionState.store(ConnectionState::CONNECTING);

        try
        {
            // Resolve server address
            asio::ip::udp::resolver resolver(*m_ioContext);
            auto endpoints = resolver.resolve(host, std::to_string(port));
            m_serverEndpoint = *endpoints.begin();

            // Open socket
            m_socket->open(asio::ip::udp::v4());

            m_running.store(true);

            // Start network thread
            m_networkThread = std::make_unique<std::thread>(&AsioClient::networkThreadLoop, this);

            // Start receiving
            startReceive();

            // Send connect packet
            sendConnect();

            m_lastServerResponse = std::chrono::steady_clock::now();

            utl::Logger::log("AsioClient: Connecting to " + host + ":" + std::to_string(port), utl::LogLevel::INFO);
        }
        catch (const std::exception &e)
        {
            m_connectionState.store(ConnectionState::DISCONNECTED);
            m_running.store(false);
            utl::Logger::log("AsioClient: Failed to connect - " + std::string(e.what()), utl::LogLevel::WARNING);
            throw;
        }
    }

    void AsioClient::disconnect()
    {
        if (m_connectionState.load() == ConnectionState::DISCONNECTED)
        {
            return;
        }

        m_connectionState.store(ConnectionState::DISCONNECTING);

        // Send disconnect packet if connected
        if (m_sessionId != 0)
        {
            sendDisconnect();
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for packet to be sent
        }

        m_running.store(false);

        // Stop io_context
        if (m_ioContext)
        {
            m_ioContext->stop();
        }

        // Wait for network thread
        if (m_networkThread && m_networkThread->joinable())
        {
            m_networkThread->join();
        }

        // Close socket
        if (m_socket && m_socket->is_open())
        {
            m_socket->close();
        }

        // Reset state
        m_connectionState.store(ConnectionState::DISCONNECTED);
        m_sessionId = 0;
        m_serverTickRate = 60;
        m_latency = 0;

        // Clear send queue
        {
            std::lock_guard<std::mutex> lock(m_sendQueueMutex);
            while (!m_sendQueue.empty())
            {
                m_sendQueue.pop();
            }
        }

        utl::Logger::log("AsioClient: Disconnected", utl::LogLevel::INFO);
    }

    void AsioClient::update()
    {
        if (!m_running.load())
        {
            return;
        }
        // Process EventBus events
        processBusEvent();

        utl::Logger::log("[CLIENT DEBUG] AsioClient::update() called\n", utl::LogLevel::INFO);

        // Process send queue
        processSendQueue();

        // Update connection management
        updateConnectionManagement();
    }

    bool AsioClient::isConnected() const { return m_connectionState.load() == ConnectionState::CONNECTED; }

    ConnectionState AsioClient::getConnectionState() const { return m_connectionState.load(); }

    std::uint32_t AsioClient::getSessionId() const { return m_sessionId; }

    std::uint16_t AsioClient::getServerTickRate() const { return m_serverTickRate; }

    std::uint32_t AsioClient::getLatency() const { return m_latency; }

    void AsioClient::setPlayerName(const std::string &playerName)
    {
        m_playerName = playerName;
        utl::Logger::log("AsioClient: Player name set to " + playerName, utl::LogLevel::INFO);
    }

    void AsioClient::setClientCapabilities(std::uint32_t caps)
    {
        m_clientCaps = caps;
        utl::Logger::log("AsioClient: Client capabilities set to " + std::to_string(caps), utl::LogLevel::INFO);
    }

    void AsioClient::sendToServer(const std::vector<std::uint8_t> &data, bool reliable)
    {
        if (m_connectionState.load() != ConnectionState::CONNECTED)
        {
            utl::Logger::log("AsioClient: Attempted to send while not connected", utl::LogLevel::WARNING);
            return;
        }

        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        m_sendQueue.emplace(data, reliable);
    }

    void AsioClient::setupPacketHandlers()
    {
        // CONNECT_ACCEPT handler
        m_packetHandler->onConnectAccept(
            [this](const rnp::PacketConnectAccept &packet, const rnp::PacketContext &context)
            { return handleConnectAccept(packet, context); });

        // DISCONNECT handler
        m_packetHandler->onDisconnect([this](const rnp::PacketDisconnect &packet, const rnp::PacketContext &context)
                                      { return handleDisconnect(packet, context); });

        // PONG handler
        m_packetHandler->onPong([this](const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
                                { return handlePong(packet, context); });

        // PING handler (server can ping client too)
        m_packetHandler->onPing([this](const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
                                { return handlePing(packet, context); });

        // ERROR handler
        m_packetHandler->onError([this](const rnp::PacketError &packet, const rnp::PacketContext &context)
                                 { return handleError(packet, context); });

        // WORLD_STATE handler
        m_packetHandler->onWorldState([this](const rnp::PacketWorldState &packet, const rnp::PacketContext &context)
                                      { return handleWorldState(packet, context); });

        // ENTITY_EVENT handler (for server events)
        m_packetHandler->onEntityEvent(
            [this](const std::vector<rnp::EventRecord> &events, const rnp::PacketContext &context)
            { return handleEntityEvent(events, context); });

        utl::Logger::log("AsioClient: Packet handlers initialized", utl::LogLevel::INFO);
    }

    void AsioClient::startReceive()
    {
        m_socket->async_receive_from(asio::buffer(m_recvBuffer), m_senderEndpoint,
                                     [this](std::error_code ec, std::size_t bytesReceived)
                                     {
                                         if (!ec && m_running.load())
                                         {
                                             handleReceive(bytesReceived);
                                             startReceive(); // Continue receiving
                                         }
                                         else if (m_running.load())
                                         {
                                             utl::Logger::log("AsioClient: Receive error - " + ec.message(),
                                                              utl::LogLevel::WARNING);
                                             startReceive(); // Try to continue
                                         }
                                     });
    }

    void AsioClient::handleReceive(std::size_t bytesReceived)
    {
        if (bytesReceived < sizeof(rnp::PacketHeader))
        {
            utl::Logger::log("AsioClient: Received packet too small", utl::LogLevel::WARNING);
            return;
        }

        m_lastServerResponse = std::chrono::steady_clock::now();
        m_stats.packetsReceived++;
        m_stats.bytesTransferred += bytesReceived;

        // Create packet context
        rnp::PacketContext context;
        context.receiveTime = std::chrono::steady_clock::now();
        context.senderAddress = m_senderEndpoint.address().to_string();
        context.senderPort = m_senderEndpoint.port();

        // Extract session ID from header for context
        std::vector<std::uint8_t> data(m_recvBuffer.begin(), m_recvBuffer.begin() + bytesReceived);
        if (data.size() >= sizeof(rnp::PacketHeader))
        {
            rnp::Serializer headerSerializer(data);
            rnp::PacketHeader header = headerSerializer.deserializeHeader();
            context.sessionId = header.sessionId;
        }

        // Process packet
        rnp::HandlerResult result = m_packetHandler->processPacket(data, context);
        if (result != rnp::HandlerResult::SUCCESS)
        {
            utl::Logger::log("AsioClient: Packet processing failed with result " +
                                 std::to_string(static_cast<int>(result)),
                             utl::LogLevel::WARNING);
        }
    }

    void AsioClient::networkThreadLoop()
    {
        utl::Logger::log("AsioClient: Network thread started", utl::LogLevel::INFO);

        while (m_running.load())
        {
            try
            {
                m_ioContext->run_for(std::chrono::milliseconds(100));
                if (!m_running.load())
                {
                    break;
                }
                m_ioContext->restart();
            }
            catch (const std::exception &e)
            {
                utl::Logger::log("AsioClient: Network thread exception - " + std::string(e.what()),
                                 utl::LogLevel::WARNING);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        utl::Logger::log("AsioClient: Network thread stopped", utl::LogLevel::INFO);
    }

    void AsioClient::sendPacketImmediate(const std::vector<std::uint8_t> &data)
    {
        if (!m_socket || !m_socket->is_open())
        {
            return;
        }

        try
        {
            m_socket->send_to(asio::buffer(data), m_serverEndpoint);
            m_stats.packetsSent++;
            m_stats.bytesTransferred += data.size();
        }
        catch (const std::exception &e)
        {
            utl::Logger::log("AsioClient: Failed to send packet - " + std::string(e.what()), utl::LogLevel::WARNING);
        }
    }

    rnp::HandlerResult AsioClient::handleConnectAccept(const rnp::PacketConnectAccept &packet,
                                                       const rnp::PacketContext &context)
    {
        m_sessionId = packet.sessionId;
        m_serverTickRate = packet.tickRateHz;
        m_connectionState.store(ConnectionState::CONNECTED);

        utl::Logger::log("AsioClient: Connection accepted - Session ID: " + std::to_string(m_sessionId) +
                             ", Tick rate: " + std::to_string(m_serverTickRate) + "Hz",
                         utl::LogLevel::INFO);

        rnp::Serializer serializer;
        m_eventBus.publish(utl::EventType::CONNECTION_ACCEPTED, serializer.getData(), m_componentId,
                           utl::RENDERING_ENGINE); // GameClient ID
        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioClient::handleDisconnect(const rnp::PacketDisconnect &packet,
                                                    const rnp::PacketContext &context)
    {
        auto reason = static_cast<rnp::DisconnectReason>(packet.reasonCode);
        std::string reasonStr = "Unknown";

        switch (reason)
        {
            case rnp::DisconnectReason::CLIENT_REQUEST:
                reasonStr = "Client request";
                break;
            case rnp::DisconnectReason::TIMEOUT:
                reasonStr = "Timeout";
                break;
            case rnp::DisconnectReason::PROTOCOL_ERROR:
                reasonStr = "Protocol error";
                break;
            case rnp::DisconnectReason::SERVER_SHUTDOWN:
                reasonStr = "Server shutdown";
                break;
            case rnp::DisconnectReason::SERVER_FULL:
                reasonStr = "Server full";
                break;
            case rnp::DisconnectReason::BANNED:
                reasonStr = "Banned";
                break;
            case rnp::DisconnectReason::UNSPECIFIED:
                reasonStr = "Unspecified";
                break;
        }

        utl::Logger::log("AsioClient: Disconnected by server - Reason: " + reasonStr, utl::LogLevel::INFO);

        m_connectionState.store(ConnectionState::DISCONNECTED);
        m_sessionId = 0;

        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioClient::handlePong(const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
    {
        if (packet.nonce == m_lastPingNonce)
        {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastPingTime);
            m_latency = static_cast<std::uint32_t>(duration.count());

            utl::Logger::log("AsioClient: Ping response received - Latency: " + std::to_string(m_latency) + "ms",
                             utl::LogLevel::INFO);
        }

        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioClient::handlePing(const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
    {
        // Server is pinging us, send pong response
        sendPong(packet.nonce);
        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioClient::handleError(const rnp::PacketError &packet, const rnp::PacketContext &context)
    {
        auto errorCode = static_cast<rnp::ErrorCode>(packet.errorCode);
        std::string errorStr = "Unknown error";

        switch (errorCode)
        {
            case rnp::ErrorCode::INVALID_PAYLOAD:
                errorStr = "Invalid payload";
                break;
            case rnp::ErrorCode::UNAUTHORIZED_SESSION:
                errorStr = "Unauthorized session";
                break;
            case rnp::ErrorCode::RATE_LIMITED:
                errorStr = "Rate limited";
                break;
            case rnp::ErrorCode::INTERNAL_ERROR:
                errorStr = "Internal server error";
                break;
            default:
                errorStr = "Unknown error";
                break;
        }

        utl::Logger::log("AsioClient: Server error - " + errorStr + ": " + packet.description, utl::LogLevel::WARNING);

        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioClient::handleWorldState(const rnp::PacketWorldState &packet,
                                                    const rnp::PacketContext &context)
    {
        // Handle world state update
        utl::Logger::log("AsioClient: World state received - Tick: " + std::to_string(packet.serverTick) +
                             ", Entities: " + std::to_string(packet.entityCount),
                         utl::LogLevel::INFO);

        // Forward to GameClient via EventBus
        m_eventBus.publish(utl::EventType::WORLD_STATE_RECEIVED, packet, m_componentId,
                           utl::RENDERING_ENGINE); // GameClient ID

        return rnp::HandlerResult::SUCCESS;
    }

    void AsioClient::sendConnect()
    {
        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::CONNECT);
        header.length = sizeof(rnp::PacketConnect);
        header.flags = 0;
        header.sessionId = 0; // No session ID yet

        rnp::PacketConnect connect{};
        connect.nameLen = static_cast<std::uint8_t>(
            std::min(m_playerName.length(), static_cast<std::size_t>(connect.playerName.size() - 1)));
        std::memset(connect.playerName.data(), 0, connect.playerName.size());
        if (!m_playerName.empty())
        {
            std::memcpy(connect.playerName.data(), m_playerName.data(), connect.nameLen);
        }
        connect.clientCaps = m_clientCaps;

        serializer.serializeHeader(header);
        serializer.serializeConnect(connect);

        sendPacketImmediate(serializer.getData());
        utl::Logger::log("AsioClient: CONNECT packet sent", utl::LogLevel::INFO);
    }

    void AsioClient::sendDisconnect()
    {
        if (m_sessionId == 0)
        {
            return;
        }

        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::DISCONNECT);
        header.length = sizeof(rnp::PacketDisconnect);
        header.flags = 0;
        header.sessionId = m_sessionId;

        rnp::PacketDisconnect disconnect{};
        disconnect.reasonCode = static_cast<std::uint16_t>(rnp::DisconnectReason::CLIENT_REQUEST);

        serializer.serializeHeader(header);
        serializer.serializeDisconnect(disconnect);

        sendPacketImmediate(serializer.getData());
        utl::Logger::log("AsioClient: DISCONNECT packet sent", utl::LogLevel::INFO);
    }

    void AsioClient::sendPing()
    {
        if (m_sessionId == 0)
        {
            return;
        }

        m_lastPingNonce = generatePingNonce();
        m_lastPingTime = std::chrono::steady_clock::now();

        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::PING);
        header.length = sizeof(rnp::PacketPingPong);
        header.flags = 0;
        header.sessionId = m_sessionId;

        rnp::PacketPingPong ping{};
        ping.nonce = m_lastPingNonce;
        ping.sendTimeMs = static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(m_lastPingTime.time_since_epoch()).count());

        serializer.serializeHeader(header);
        serializer.serializePingPong(ping);

        sendPacketImmediate(serializer.getData());
    }

    void AsioClient::sendPong(std::uint32_t nonce)
    {
        if (m_sessionId == 0)
        {
            return;
        }

        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::PONG);
        header.length = sizeof(rnp::PacketPingPong);
        header.flags = 0;
        header.sessionId = m_sessionId;

        rnp::PacketPingPong pong{};
        pong.nonce = nonce;
        pong.sendTimeMs = static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count());

        serializer.serializeHeader(header);
        serializer.serializePingPong(pong);

        sendPacketImmediate(serializer.getData());
    }

    void AsioClient::processSendQueue()
    {
        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        while (!m_sendQueue.empty())
        {
            const QueuedPacket &packet = m_sendQueue.front();
            sendPacketImmediate(packet.data);
            m_sendQueue.pop();
        }
    }

    void AsioClient::updateConnectionManagement()
    {
        auto now = std::chrono::steady_clock::now();

        // Send periodic pings when connected
        if (m_connectionState.load() == ConnectionState::CONNECTED)
        {
            if (now - m_lastPingTime > m_pingInterval)
            {
                sendPing();
            }
        }

        // Check for connection timeout
        if (m_connectionState.load() == ConnectionState::CONNECTING ||
            m_connectionState.load() == ConnectionState::CONNECTED)
        {
            if (now - m_lastServerResponse > m_connectionTimeout)
            {
                utl::Logger::log("AsioClient: Connection timed out", utl::LogLevel::WARNING);
                m_connectionState.store(ConnectionState::DISCONNECTED);
                m_sessionId = 0;
            }
        }

        // Retry connection if in CONNECTING state and enough time has passed
        if (m_connectionState.load() == ConnectionState::CONNECTING)
        {
            static auto lastConnectAttempt = std::chrono::steady_clock::now();
            if (now - lastConnectAttempt > std::chrono::seconds(2))
            {
                sendConnect();
                lastConnectAttempt = now;
            }
        }
    }

    std::uint32_t AsioClient::generatePingNonce()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<std::uint32_t> dis(1, UINT32_MAX);
        return dis(gen);
    }

    void AsioClient::processBusEvent()
    {
        const auto events = m_eventBus.consumeForTarget(m_componentId);

        for (const auto &e : events)
        {
            switch (e.type)
            {
                case utl::EventType::SEND_PLAYER_INPUT:
                {
                    sendToServer(e.data);
                    break;
                }
                case utl::EventType::REQUEST_CONNECT:
                {
                    if (m_connectionState == ConnectionState::DISCONNECTED)
                    {
                        connect("127.0.0.1", 4242);
                    }
                    break;
                }
                case utl::EventType::REQUEST_DISCONNECT:
                {
                    if (m_connectionState == ConnectionState::CONNECTED)
                    {
                        disconnect();
                    }
                    break;
                }
                case utl::EventType::SEND_ENTITY_EVENT:
                {
                    // Forward entity events to server
                    sendToServer(e.data);
                    break;
                }
                default:
                {
                    utl::Logger::log("AsioClient: Unhandled event type: " + std::to_string(static_cast<int>(e.type)),
                                     utl::LogLevel::WARNING);
                    break;
                }
            }
        }
    }

    rnp::HandlerResult AsioClient::handleEntityEvent(const std::vector<rnp::EventRecord> &events,
                                                     const rnp::PacketContext &context)
    {
        utl::Logger::log("AsioClient: Received " + std::to_string(events.size()) + " entity events from server",
                         utl::LogLevel::INFO);

        m_eventBus.publish(utl::EventType::ENTITY_EVENT_RECEIVED, events, m_componentId,
                           utl::RENDERING_ENGINE); // GameClient ID

        return rnp::HandlerResult::SUCCESS;
    }

} // namespace eng
