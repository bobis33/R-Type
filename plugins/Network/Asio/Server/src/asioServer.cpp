///
/// @file AsioServer.cpp
/// @brief Asio-based implementation of INetworkServer interface
/// @namespace srv
///

#include "AsioServer/AsioServer.hpp"
#include "Utils/Event.hpp"
#include "Utils/EventBus.hpp"
#include "Utils/Logger.hpp"

#include <chrono>
#include <iostream>
#include <random>

namespace srv
{

    AsioServer::AsioServer()
        : m_ioContext(std::make_unique<asio::io_context>()),
          m_socket(std::make_unique<asio::ip::udp::socket>(*m_ioContext)), m_port(0), m_tickRate(60), m_serverCaps(0),
          m_running(false), m_started(false), m_nextSessionId(1),
          m_packetHandler(std::make_unique<rnp::HandlerPacket>()), m_pingInterval(std::chrono::seconds(5)),
          m_clientTimeout(std::chrono::seconds(30)), m_eventBus(utl::EventBus::getInstance()), m_componentId(2)
    {
        utl::Logger::log("AsioServer: Constructor called", utl::LogLevel::INFO);
        utl::Logger::log("AsioServer: Creating I/O context and socket", utl::LogLevel::INFO);
        setupPacketHandlers();
        utl::Logger::log("AsioServer: Packet handlers setup complete", utl::LogLevel::INFO);

        // EventBus integration
        m_eventBus.registerComponent(m_componentId, "Asio Server");
        m_eventBus.subscribe(m_componentId, utl::EventType::SEND_TO_CLIENT);
        m_eventBus.subscribe(m_componentId, utl::EventType::BROADCAST_WORLD_STATE);

        utl::Logger::log("AsioServer: Initialized with EventBus integration", utl::LogLevel::INFO);
    }

    AsioServer::~AsioServer()
    {
        utl::Logger::log("AsioServer: Destructor called", utl::LogLevel::INFO);
        stop();
        m_eventBus.unregisterComponent(m_componentId);
        utl::Logger::log("AsioServer: Destroyed", utl::LogLevel::INFO);
    }

    void AsioServer::init(const std::string &host, std::uint16_t port)
    {
        utl::Logger::log("AsioServer: init() called with host=" + host + ", port=" + std::to_string(port),
                         utl::LogLevel::INFO);
        m_host = host;
        m_port = port;
        utl::Logger::log("AsioServer: Configuration stored - ready to start", utl::LogLevel::INFO);
    }

    void AsioServer::start()
    {
        utl::Logger::log("AsioServer: Starting server...", utl::LogLevel::INFO);

        if (m_started.load())
        {
            utl::Logger::log("AsioServer: Already started", utl::LogLevel::WARNING);
            return;
        }

        try
        {
            utl::Logger::log("AsioServer: Creating endpoint for " + m_host + ":" + std::to_string(m_port),
                             utl::LogLevel::INFO);

            // Create endpoint
            asio::ip::udp::endpoint endpoint(asio::ip::make_address(m_host), m_port);
            utl::Logger::log("AsioServer: Endpoint created successfully", utl::LogLevel::INFO);

            // Bind socket
            utl::Logger::log("AsioServer: Opening socket...", utl::LogLevel::INFO);
            m_socket->open(endpoint.protocol());
            utl::Logger::log("AsioServer: Setting socket options...", utl::LogLevel::INFO);
            m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
            utl::Logger::log("AsioServer: Binding socket to endpoint...", utl::LogLevel::INFO);
            m_socket->bind(endpoint);
            utl::Logger::log("AsioServer: Socket bound successfully", utl::LogLevel::INFO);

            m_running.store(true);
            m_started.store(true);

            // Start network thread
            utl::Logger::log("AsioServer: Starting network thread...", utl::LogLevel::INFO);
            m_networkThread = std::make_unique<std::thread>(&AsioServer::networkThreadLoop, this);

            // Start receiving
            utl::Logger::log("AsioServer: Starting packet reception...", utl::LogLevel::INFO);
            startReceive();

            utl::Logger::log("AsioServer: Started on " + m_host + ":" + std::to_string(m_port), utl::LogLevel::INFO);
        }
        catch (const std::exception &e)
        {
            m_running.store(false);
            m_started.store(false);
            utl::Logger::log("AsioServer: Failed to start - " + std::string(e.what()), utl::LogLevel::WARNING);
            throw;
        }
    }

    void AsioServer::stop()
    {
        if (!m_started.load())
        {
            return;
        }

        m_running.store(false);
        m_started.store(false);

        // Send disconnect to all clients
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (auto &[sessionId, client] : m_clients)
            {
                if (client.isConnected)
                {
                    rnp::Serializer serializer;
                    rnp::PacketHeader header{};
                    header.type = static_cast<std::uint8_t>(rnp::PacketType::DISCONNECT);
                    header.length = sizeof(rnp::PacketDisconnect);
                    header.flags = 0;
                    header.sessionId = sessionId;

                    rnp::PacketDisconnect disconnect{};
                    disconnect.reasonCode = static_cast<std::uint16_t>(rnp::DisconnectReason::SERVER_SHUTDOWN);

                    serializer.serializeHeader(header);
                    serializer.serializeDisconnect(disconnect);

                    sendPacketImmediate(serializer.getData(), client.endpoint);
                }
            }
        }

        // Stop io_context safely
        if (m_ioContext)
        {
            m_ioContext->stop();
        }

        // Wait for network thread with timeout
        if (m_networkThread && m_networkThread->joinable())
        {
            auto joinStart = std::chrono::steady_clock::now();
            bool joined = false;

            while (
                !joined &&
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - joinStart).count() <
                    5)
            {
                if (m_networkThread->joinable())
                {
                    m_networkThread->join();
                    joined = true;
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }

            if (!joined)
            {
                utl::Logger::log("AsioServer: Network thread join timeout, detaching", utl::LogLevel::WARNING);
                m_networkThread->detach();
            }
        }

        // Close socket safely
        {
            std::lock_guard<std::mutex> socketLock(m_socketMutex);
            if (m_socket && m_socket->is_open())
            {
                try
                {
                    m_socket->close();
                }
                catch (const std::exception &e)
                {
                    utl::Logger::log("AsioServer: Exception closing socket - " + std::string(e.what()),
                                     utl::LogLevel::WARNING);
                }
            }
        }

        // Clear clients
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clients.clear();
            m_endpointToSession.clear();
        }

        utl::Logger::log("AsioServer: Stopped", utl::LogLevel::INFO);
    }

    void AsioServer::update()
    {
        if (!m_running.load())
        {
            static bool loggedNotRunning = false;
            if (!loggedNotRunning)
            {
                utl::Logger::log("AsioServer: Update called but server not running", utl::LogLevel::WARNING);
                loggedNotRunning = true;
            }
            return;
        }

        // Process EventBus events
        processEventBusEvents();

        // Process send queue
        processSendQueue();

        // Update client management (timeouts, pings)
        updateClientManagement();
    }

    void AsioServer::sendToClient(std::uint32_t sessionId, const std::vector<std::uint8_t> &data, bool reliable)
    {
        std::lock_guard<std::mutex> clientLock(m_clientsMutex);
        auto it = m_clients.find(sessionId);
        if (it == m_clients.end() || !it->second.isConnected)
        {
            utl::Logger::log("AsioServer: Attempted to send to invalid session " + std::to_string(sessionId),
                             utl::LogLevel::WARNING);
            return;
        }

        std::lock_guard<std::mutex> queueLock(m_sendQueueMutex);
        m_sendQueue.emplace(data, it->second.endpoint, reliable);
    }

    void AsioServer::sendToAllClients(const std::vector<std::uint8_t> &data, bool reliable)
    {
        std::lock_guard<std::mutex> clientLock(m_clientsMutex);
        std::lock_guard<std::mutex> queueLock(m_sendQueueMutex);

        for (const auto &[sessionId, client] : m_clients)
        {
            if (client.isConnected)
            {
                m_sendQueue.emplace(data, client.endpoint, reliable);
            }
        }
    }

    void AsioServer::disconnectClient(std::uint32_t sessionId)
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        auto it = m_clients.find(sessionId);
        if (it == m_clients.end())
        {
            return;
        }

        // Send disconnect packet
        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::DISCONNECT);
        header.length = sizeof(rnp::PacketDisconnect);
        header.flags = 0;
        header.sessionId = sessionId;

        rnp::PacketDisconnect disconnect{};
        disconnect.reasonCode = static_cast<std::uint16_t>(rnp::DisconnectReason::CLIENT_REQUEST);

        serializer.serializeHeader(header);
        serializer.serializeDisconnect(disconnect);

        sendPacketImmediate(serializer.getData(), it->second.endpoint);

        // Remove from endpoint mapping
        std::string endpointStr = endpointToString(it->second.endpoint);
        m_endpointToSession.erase(endpointStr);

        // Remove client
        m_clients.erase(it);

        m_packetHandler->clearSession(sessionId);

        utl::Logger::log("AsioServer: Disconnected client " + std::to_string(sessionId), utl::LogLevel::INFO);
    }

    std::size_t AsioServer::getClientCount() const
    {
        std::lock_guard<std::mutex> clientLock(m_clientsMutex);
        return std::count_if(m_clients.begin(), m_clients.end(),
                             [](const auto &pair) { return pair.second.isConnected; });
    }

    std::vector<std::uint32_t> AsioServer::getConnectedSessions() const
    {
        std::lock_guard<std::mutex> clientLock(m_clientsMutex);
        std::vector<std::uint32_t> sessions;
        for (const auto &[sessionId, client] : m_clients)
        {
            if (client.isConnected)
            {
                sessions.push_back(sessionId);
            }
        }
        return sessions;
    }

    bool AsioServer::isRunning() const { return m_running.load(); }

    void AsioServer::setTickRate(std::uint16_t tickRate)
    {
        m_tickRate = tickRate;
        utl::Logger::log("AsioServer: Tick rate set to " + std::to_string(tickRate), utl::LogLevel::INFO);
    }

    void AsioServer::setServerCapabilities(std::uint32_t caps)
    {
        m_serverCaps = caps;
        utl::Logger::log("AsioServer: Server capabilities set to " + std::to_string(caps), utl::LogLevel::INFO);
    }

    void AsioServer::setupPacketHandlers()
    {
        // CONNECT handler
        m_packetHandler->onConnect([this](const rnp::PacketConnect &packet, const rnp::PacketContext &context)
                                   { return handleConnect(packet, context); });

        // DISCONNECT handler
        m_packetHandler->onDisconnect([this](const rnp::PacketDisconnect &packet, const rnp::PacketContext &context)
                                      { return handleDisconnect(packet, context); });

        // PING handler
        m_packetHandler->onPing([this](const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
                                { return handlePing(packet, context); });

        // PONG handler
        m_packetHandler->onPong([this](const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
                                { return handlePong(packet, context); });

        // ENTITY_EVENT handler (for player inputs)
        m_packetHandler->onEntityEvent(
            [this](const std::vector<rnp::EventRecord> &events, const rnp::PacketContext &context)
            { return handleEntityEvent(events, context); });

        utl::Logger::log("AsioServer: Packet handlers initialized", utl::LogLevel::INFO);
    }

    void AsioServer::startReceive()
    {
        // Check if we should continue receiving
        if (!m_running.load() || !m_socket || !m_socket->is_open())
        {
            utl::Logger::log("AsioServer: Cannot start receive - server not running or socket closed",
                             utl::LogLevel::INFO);
            return;
        }

        utl::Logger::log("AsioServer: Setting up async receive...", utl::LogLevel::INFO);
        m_socket->async_receive_from(
            asio::buffer(m_recvBuffer), m_senderEndpoint,
            [this](std::error_code ec, std::size_t bytesReceived)
            {
                if (!ec && m_running.load())
                {
                    utl::Logger::log("AsioServer: Received " + std::to_string(bytesReceived) + " bytes from " +
                                         m_senderEndpoint.address().to_string() + ":" +
                                         std::to_string(m_senderEndpoint.port()),
                                     utl::LogLevel::INFO);
                    handleReceive(bytesReceived);
                    startReceive(); // Continue receiving
                }
                else if (m_running.load() && ec != asio::error::operation_aborted)
                {
                    utl::Logger::log("AsioServer: Receive error - " + ec.message(), utl::LogLevel::WARNING);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    startReceive(); // Try to continue
                }
                else
                {
                    utl::Logger::log("AsioServer: Server stopped or operation aborted, ending receive loop",
                                     utl::LogLevel::INFO);
                }
            });
    }

    void AsioServer::handleReceive(std::size_t bytesReceived)
    {
        utl::Logger::log("AsioServer: Processing received packet of " + std::to_string(bytesReceived) + " bytes",
                         utl::LogLevel::INFO);

        if (bytesReceived < sizeof(rnp::PacketHeader))
        {
            utl::Logger::log("AsioServer: Received packet too small (need " +
                                 std::to_string(sizeof(rnp::PacketHeader)) + " bytes, got " +
                                 std::to_string(bytesReceived) + ")",
                             utl::LogLevel::WARNING);
            return;
        }

        // Create packet context
        rnp::PacketContext context;
        context.receiveTime = std::chrono::steady_clock::now();
        context.senderAddress = m_senderEndpoint.address().to_string();
        context.senderPort = m_senderEndpoint.port();

        utl::Logger::log("AsioServer: Packet from " + context.senderAddress + ":" + std::to_string(context.senderPort),
                         utl::LogLevel::INFO);

        // Extract session ID from header for context
        std::vector<std::uint8_t> data(m_recvBuffer.begin(), m_recvBuffer.begin() + bytesReceived);
        if (data.size() >= sizeof(rnp::PacketHeader))
        {
            rnp::Serializer headerSerializer(data);
            rnp::PacketHeader header = headerSerializer.deserializeHeader();
            context.sessionId = header.sessionId;
            utl::Logger::log("AsioServer: Packet type: " + std::to_string(static_cast<int>(header.type)) +
                                 ", Session ID: " + std::to_string(header.sessionId),
                             utl::LogLevel::INFO);
        }

        // Process packet
        utl::Logger::log("AsioServer: Processing packet...", utl::LogLevel::INFO);
        rnp::HandlerResult result = m_packetHandler->processPacket(data, context);
        if (result != rnp::HandlerResult::SUCCESS)
        {
            utl::Logger::log("AsioServer: Packet processing failed with result " +
                                 std::to_string(static_cast<int>(result)),
                             utl::LogLevel::WARNING);
        }
        else
        {
            utl::Logger::log("AsioServer: Packet processed successfully", utl::LogLevel::INFO);
        }
    }

    void AsioServer::networkThreadLoop()
    {
        utl::Logger::log("AsioServer: Network thread started", utl::LogLevel::INFO);

        while (m_running.load())
        {
            try
            {
                // Run I/O context and check if we should continue
                if (!m_running.load())
                {
                    break;
                }

                size_t handlersRun = m_ioContext->run_one();
                if (handlersRun == 0)
                {
                    // No handlers to run, sleep briefly to avoid busy waiting
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            catch (const std::exception &e)
            {
                if (m_running.load())
                {
                    utl::Logger::log("AsioServer: Network thread exception - " + std::string(e.what()),
                                     utl::LogLevel::WARNING);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }

        utl::Logger::log("AsioServer: Network thread stopped", utl::LogLevel::INFO);
    }

    std::uint32_t AsioServer::generateSessionId()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::uint32_t> dis(1, UINT32_MAX);

        std::uint32_t sessionId = 0;
        while (sessionId == 0 || m_clients.find(sessionId) != m_clients.end())
        {
            sessionId = dis(gen);
        }

        return sessionId;
    }

    std::string AsioServer::endpointToString(const asio::ip::udp::endpoint &endpoint) const
    {
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    void AsioServer::sendPacketImmediate(const std::vector<std::uint8_t> &data,
                                         const asio::ip::udp::endpoint &destination)
    {
        utl::Logger::log("AsioServer: Sending " + std::to_string(data.size()) + " bytes to " +
                             destination.address().to_string() + ":" + std::to_string(destination.port()),
                         utl::LogLevel::INFO);

        // Protect socket access with mutex
        std::lock_guard<std::mutex> socketLock(m_socketMutex);

        if (!m_socket || !m_socket->is_open() || !m_running.load())
        {
            utl::Logger::log("AsioServer: Cannot send - socket not open or server not running", utl::LogLevel::WARNING);
            return;
        }

        try
        {
            size_t bytesSent = m_socket->send_to(asio::buffer(data), destination);
            utl::Logger::log("AsioServer: Successfully sent " + std::to_string(bytesSent) + " bytes",
                             utl::LogLevel::INFO);
        }
        catch (const std::exception &e)
        {
            if (m_running.load())
            {
                utl::Logger::log("AsioServer: Failed to send packet - " + std::string(e.what()),
                                 utl::LogLevel::WARNING);
            }
        }
    }

    rnp::HandlerResult AsioServer::handleConnect(const rnp::PacketConnect &packet, const rnp::PacketContext &context)
    {
        utl::Logger::log("AsioServer: Handling CONNECT packet from " + context.senderAddress + ":" +
                             std::to_string(context.senderPort),
                         utl::LogLevel::INFO);

        std::lock_guard<std::mutex> lock(m_clientsMutex);
        // Check if server is full (avoid getClientCount() to prevent deadlock)
        size_t currentClientCount =
            std::count_if(m_clients.begin(), m_clients.end(), [](const auto &pair) { return pair.second.isConnected; });
        utl::Logger::log("AsioServer: Current client count: " + std::to_string(currentClientCount) + "/" +
                             std::to_string(MAX_CLIENTS),
                         utl::LogLevel::INFO);

        if (currentClientCount >= MAX_CLIENTS)
        {
            utl::Logger::log("AsioServer: Server full, rejecting connection", utl::LogLevel::WARNING);
            sendError(rnp::ErrorCode::RATE_LIMITED, "Server full", m_senderEndpoint, 0);
            return rnp::HandlerResult::SUCCESS;
        }

        // Check if client already exists
        std::string endpointStr = endpointToString(m_senderEndpoint);
        auto existingIt = m_endpointToSession.find(endpointStr);
        if (existingIt != m_endpointToSession.end())
        {
            utl::Logger::log("AsioServer: Client already connected, resending accept (Session ID: " +
                                 std::to_string(existingIt->second) + ")",
                             utl::LogLevel::INFO);
            // Client already connected, resend accept
            sendConnectAccept(existingIt->second, m_senderEndpoint);
            return rnp::HandlerResult::SUCCESS;
        }

        // Create new session
        std::uint32_t sessionId = generateSessionId();
        utl::Logger::log("AsioServer: Creating new session with ID: " + std::to_string(sessionId), utl::LogLevel::INFO);

        ClientSession &client = m_clients[sessionId];
        client.sessionId = sessionId;
        client.endpoint = m_senderEndpoint;
        client.lastSeen = context.receiveTime;
        client.playerName = std::string(packet.playerName.data(), packet.nameLen);
        client.clientCaps = packet.clientCaps;
        client.isConnected = true;

        m_endpointToSession[endpointStr] = sessionId;

        utl::Logger::log("AsioServer: Player name: '" + client.playerName +
                             "', Caps: " + std::to_string(client.clientCaps),
                         utl::LogLevel::INFO);

        // Send accept response
        utl::Logger::log("AsioServer: Sending CONNECT_ACCEPT to session " + std::to_string(sessionId),
                         utl::LogLevel::INFO);
        sendConnectAccept(sessionId, m_senderEndpoint);

        utl::Logger::log("AsioServer: Client connected - " + client.playerName + " (ID: " + std::to_string(sessionId) +
                             ")",
                         utl::LogLevel::INFO);

        // Publier vers GameServer
        m_eventBus.publish(utl::EventType::PLAYER_CONNECTED, sessionId, m_componentId,
                           4000); // GameServer ID

        utl::Logger::log(
            "AsioServer: Nouvelle connexion publiée vers GameServer (sessionId: " + std::to_string(sessionId) + ")",
            utl::LogLevel::INFO);

        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioServer::handleDisconnect(const rnp::PacketDisconnect &packet,
                                                    const rnp::PacketContext &context)
    {
        utl::Logger::log("AsioServer: Handling DISCONNECT packet from session " + std::to_string(context.sessionId) +
                             " from " + context.senderAddress + ":" + std::to_string(context.senderPort),
                         utl::LogLevel::INFO);

        std::lock_guard<std::mutex> lock(m_clientsMutex);

        auto it = m_clients.find(context.sessionId);
        if (it != m_clients.end())
        {
            utl::Logger::log("AsioServer: Client disconnected - " + it->second.playerName +
                                 " (ID: " + std::to_string(context.sessionId) + ")" +
                                 ", Reason: " + std::to_string(static_cast<int>(packet.reasonCode)),
                             utl::LogLevel::INFO);

            // Remove from endpoint mapping
            std::string endpointStr = endpointToString(it->second.endpoint);
            utl::Logger::log("AsioServer: Removing endpoint mapping: " + endpointStr, utl::LogLevel::INFO);
            m_endpointToSession.erase(endpointStr);

            // Remove client
            utl::Logger::log("AsioServer: Removing client from session list", utl::LogLevel::INFO);
            m_clients.erase(it);
        }
        else
        {
            utl::Logger::log("AsioServer: DISCONNECT from unknown session " + std::to_string(context.sessionId),
                             utl::LogLevel::WARNING);
        }

        // Publier vers GameServer
        m_eventBus.publish(utl::EventType::PLAYER_DISCONNECTED, context.sessionId, m_componentId,
                           4000); // GameServer ID

        utl::Logger::log(
            "AsioServer: Déconnexion publiée vers GameServer (sessionId: " + std::to_string(context.sessionId) + ")",
            utl::LogLevel::INFO);

        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioServer::handlePing(const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
    {
        utl::Logger::log("AsioServer: Handling PING from session " + std::to_string(context.sessionId) +
                             " with nonce " + std::to_string(packet.nonce),
                         utl::LogLevel::INFO);

        // Update client last seen time
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            auto it = m_clients.find(context.sessionId);
            if (it != m_clients.end())
            {
                it->second.lastSeen = context.receiveTime;
                utl::Logger::log("AsioServer: Updated last seen time for session " + std::to_string(context.sessionId),
                                 utl::LogLevel::INFO);
            }
            else
            {
                utl::Logger::log("AsioServer: PING from unknown session " + std::to_string(context.sessionId),
                                 utl::LogLevel::WARNING);
            }
        }

        // Send pong response
        utl::Logger::log("AsioServer: Sending PONG response to session " + std::to_string(context.sessionId),
                         utl::LogLevel::INFO);
        sendPong(packet.nonce, m_senderEndpoint, context.sessionId);
        return rnp::HandlerResult::SUCCESS;
    }

    rnp::HandlerResult AsioServer::handlePong(const rnp::PacketPingPong &packet, const rnp::PacketContext &context)
    {
        utl::Logger::log("AsioServer: Handling PONG from session " + std::to_string(context.sessionId) +
                             " with nonce " + std::to_string(packet.nonce),
                         utl::LogLevel::INFO);

        // Update client latency and last seen time
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            auto it = m_clients.find(context.sessionId);
            if (it != m_clients.end())
            {
                it->second.lastSeen = context.receiveTime;
                // Calculate latency based on ping time
                auto now = std::chrono::steady_clock::now();
                auto pingTime = std::chrono::milliseconds(packet.sendTimeMs);
                auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
                it->second.latency = static_cast<std::uint32_t>((currentTime - pingTime).count());

                utl::Logger::log("AsioServer: Updated client " + std::to_string(context.sessionId) +
                                     " latency: " + std::to_string(it->second.latency) + "ms",
                                 utl::LogLevel::INFO);
            }
            else
            {
                utl::Logger::log("AsioServer: PONG from unknown session " + std::to_string(context.sessionId),
                                 utl::LogLevel::WARNING);
            }
        }

        return rnp::HandlerResult::SUCCESS;
    }

    void AsioServer::sendPong(std::uint32_t nonce, const asio::ip::udp::endpoint &destination, std::uint32_t sessionId)
    {
        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::PONG);
        header.length = sizeof(rnp::PacketPingPong);
        header.flags = 0;
        header.sessionId = sessionId;

        rnp::PacketPingPong pong{};
        pong.nonce = nonce;
        pong.sendTimeMs = static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count());

        serializer.serializeHeader(header);
        serializer.serializePingPong(pong);

        sendPacketImmediate(serializer.getData(), destination);
    }

    void AsioServer::sendConnectAccept(std::uint32_t sessionId, const asio::ip::udp::endpoint &destination)
    {
        utl::Logger::log("AsioServer: Preparing CONNECT_ACCEPT packet for session " + std::to_string(sessionId) +
                             " to " + destination.address().to_string() + ":" + std::to_string(destination.port()),
                         utl::LogLevel::INFO);

        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::CONNECT_ACCEPT);
        header.length = sizeof(rnp::PacketConnectAccept);
        header.flags = 0;
        header.sessionId = sessionId;

        rnp::PacketConnectAccept accept{};
        accept.sessionId = sessionId;
        accept.tickRateHz = m_tickRate;
        accept.mtuPayloadBytes = rnp::MAX_PAYLOAD;
        accept.serverCaps = m_serverCaps;

        serializer.serializeHeader(header);
        serializer.serializeConnectAccept(accept);

        utl::Logger::log("AsioServer: Sending CONNECT_ACCEPT packet (" + std::to_string(serializer.getData().size()) +
                             " bytes)",
                         utl::LogLevel::INFO);
        sendPacketImmediate(serializer.getData(), destination);
    }

    void AsioServer::sendError(rnp::ErrorCode errorCode, const std::string &description,
                               const asio::ip::udp::endpoint &destination, std::uint32_t sessionId)
    {
        rnp::Serializer serializer;
        rnp::PacketHeader header{};
        header.type = static_cast<std::uint8_t>(rnp::PacketType::PACKET_ERROR);
        header.length = static_cast<std::uint16_t>(sizeof(rnp::PacketError) + description.length());
        header.flags = 0;
        header.sessionId = sessionId;

        rnp::PacketError error{};
        error.errorCode = static_cast<std::uint16_t>(errorCode);
        error.msgLen = static_cast<std::uint16_t>(description.length());
        error.description = description;

        serializer.serializeHeader(header);
        serializer.serializeError(error);

        sendPacketImmediate(serializer.getData(), destination);
    }

    void AsioServer::updateClientManagement()
    {
        auto now = std::chrono::steady_clock::now();

        // Send pings periodically
        if (now - m_lastPingTime > m_pingInterval)
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (auto &[sessionId, client] : m_clients)
            {
                if (client.isConnected)
                {
                    // Send ping
                    rnp::Serializer serializer;
                    rnp::PacketHeader header{};
                    header.type = static_cast<std::uint8_t>(rnp::PacketType::PING);
                    header.length = sizeof(rnp::PacketPingPong);
                    header.flags = 0;
                    header.sessionId = sessionId;

                    rnp::PacketPingPong ping{};
                    ping.nonce = ++client.lastPingSent;
                    ping.sendTimeMs = static_cast<std::uint32_t>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

                    serializer.serializeHeader(header);
                    serializer.serializePingPong(ping);

                    std::lock_guard<std::mutex> queueLock(m_sendQueueMutex);
                    m_sendQueue.emplace(serializer.getData(), client.endpoint, false);
                }
            }
            m_lastPingTime = now;
        }

        // Check for timeouts
        std::vector<std::uint32_t> timedOutClients;
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (const auto &[sessionId, client] : m_clients)
            {
                if (client.isConnected && (now - client.lastSeen) > m_clientTimeout)
                {
                    timedOutClients.push_back(sessionId);
                }
            }
        }

        // Disconnect timed out clients
        for (std::uint32_t sessionId : timedOutClients)
        {
            utl::Logger::log("AsioServer: Client " + std::to_string(sessionId) + " timed out", utl::LogLevel::INFO);
            disconnectClient(sessionId);
        }
    }

    void AsioServer::processSendQueue()
    {
        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        while (!m_sendQueue.empty())
        {
            const QueuedPacket &packet = m_sendQueue.front();
            sendPacketImmediate(packet.data, packet.destination);
            m_sendQueue.pop();
        }
    }

    void AsioServer::processEventBusEvents()
    {
        // Consommer les événements du GameServer
        auto events = m_eventBus.consumeForTarget(m_componentId);

        for (const auto &event : events)
        {
            switch (event.type)
            {
                case utl::EventType::SEND_TO_CLIENT:
                    handleSendToClientEvent(event);
                    break;

                case utl::EventType::BROADCAST_WORLD_STATE:
                    handleBroadcastEvent(event);
                    break;

                default:
                    utl::Logger::log("AsioServer: Événement non géré: " +
                                         std::to_string(static_cast<uint32_t>(event.type)),
                                     utl::LogLevel::WARNING);
                    break;
            }
        }
    }

    void AsioServer::handleSendToClientEvent(const utl::Event &event)
    {
        // Extraire l'ID du client cible et les données
        if (event.data.size() >= sizeof(uint32_t))
        {
            uint32_t sessionId = 0;
            std::memcpy(&sessionId, event.data.data(), sizeof(uint32_t));

            std::vector<uint8_t> messageData(event.data.begin() + sizeof(uint32_t), event.data.end());

            // Utiliser la méthode existante
            sendToClient(sessionId, messageData);

            utl::Logger::log("AsioServer: Message envoye au client " + std::to_string(sessionId) +
                                 " (taille: " + std::to_string(messageData.size()) + " bytes)",
                             utl::LogLevel::INFO);
        }
    }

    void AsioServer::handleBroadcastEvent(const utl::Event &event)
    {
        // Utiliser la méthode existante pour broadcaster
        sendToAllClients(event.data);

        utl::Logger::log(
            "AsioServer: Message diffuse a tous les clients (taille: " + std::to_string(event.data.size()) + " bytes)",
            utl::LogLevel::INFO);
    }

    rnp::HandlerResult AsioServer::handleEntityEvent(const std::vector<rnp::EventRecord> &events,
                                                     const rnp::PacketContext &context)
    {
        // Filtrer les événements d'input et les publier vers GameServer
        for (const auto &eventRecord : events)
        {
            if (eventRecord.type == rnp::EventType::INPUT)
            {
                m_eventBus.publish(utl::EventType::PLAYER_INPUT_RECEIVED, eventRecord, m_componentId,
                                   4000); // GameServer ID

                utl::Logger::log("AsioServer: Input joueur publie vers GameServer (sessionId: " +
                                     std::to_string(context.sessionId) + ")",
                                 utl::LogLevel::INFO);
            }
            else
            {
                m_eventBus.publish(utl::EventType::ENTITY_EVENT_RECEIVED, eventRecord, m_componentId,
                                   4000); // GameServer ID

                utl::Logger::log("AsioServer: Evenement entite publie vers GameServer (type: " +
                                     std::to_string(static_cast<uint8_t>(eventRecord.type)) + ")",
                                 utl::LogLevel::INFO);
            }
        }

        return rnp::HandlerResult::SUCCESS;
    }

} // namespace srv
