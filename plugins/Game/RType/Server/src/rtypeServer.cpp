#include "RTypeServer/RTypeServer.hpp"
#include "ECS/Component.hpp"
#include "Utils/EventBus.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include <algorithm>

gme::RTypeServer::RTypeServer() : m_eventBus(utl::EventBus::getInstance())
{
    // Register this component with the event bus
    m_eventBus.registerComponent(utl::GAME_LOGIC, "RTypeServer");
    
    // Subscribe to SERVER_START event to create player entities
    m_eventBus.subscribe(utl::GAME_LOGIC, utl::EventType::SERVER_START);
    
    // Subscribe to PLAYER_INPUT_RECEIVED to process player inputs
    m_eventBus.subscribe(utl::GAME_LOGIC, utl::EventType::PLAYER_INPUT_RECEIVED);
}

void gme::RTypeServer::start(/* vector<clientId> clientIds*/)
{
    m_gameState = State::PLAYING;

    utl::Logger::log("RTypeServer: Start called", utl::LogLevel::INFO);
    
    // SERVER_START events will be processed in update()
}

void gme::RTypeServer::stop() 
{ 
    m_gameState = State::LOSE;
    m_playerEntities.clear();
    m_projectileEntities.clear();
}

void gme::RTypeServer::processServerStartEvents()
{
    // Already handled in update() now
}

void gme::RTypeServer::update(const float deltaTime)
{
    m_lastBroadcastTime += deltaTime;
    
    // Process all events (SERVER_START and PLAYER_INPUT_RECEIVED)
    auto events = m_eventBus.consumeForTarget(utl::GAME_LOGIC);
    for (const auto &event : events)
    {
        if (event.type == utl::EventType::SERVER_START)
        {
            // Create player entities from sessionIds
            if (event.data.size() >= sizeof(std::uint32_t))
            {
                size_t playerCount = event.data.size() / sizeof(std::uint32_t);
                for (size_t i = 0; i < playerCount; ++i)
                {
                    std::uint32_t sessionId;
                    std::memcpy(&sessionId, event.data.data() + i * sizeof(std::uint32_t), sizeof(std::uint32_t));
                    
                    if (m_playerEntities.find(sessionId) == m_playerEntities.end())
                    {
                        ecs::Entity playerEntity = m_registry.createEntity()
                                                            .with<ecs::Transform>("player_transform_" + std::to_string(sessionId),
                                                                                  200.F + (i * 200.F), 100.F, 0.F)
                                                            .with<ecs::Velocity>("player_velocity_" + std::to_string(sessionId), 0.F, 0.F)
                                                            .with<ecs::Player>("player_" + std::to_string(sessionId), true)
                                                            .build();
                        m_playerEntities[sessionId] = playerEntity;
                        utl::Logger::log("RTypeServer: Created player entity for sessionId " + std::to_string(sessionId) + " at position " + std::to_string(200.F + (i * 200.F)), utl::LogLevel::INFO);
                    }
                }
            }
        }
        else if (event.type == utl::EventType::PLAYER_INPUT_RECEIVED)
        {
            // Process player input directly here
            try
            {
                if (event.data.size() >= 5)
                {
                    bool up = event.data[0] != 0;
                    bool down = event.data[1] != 0;
                    bool left = event.data[2] != 0;
                    bool right = event.data[3] != 0;
                    bool shoot = event.data[4] != 0;

                    std::uint32_t sessionId = event.sourceId;
                    
                    utl::Logger::log("RTypeServer: Received input from sessionId " + std::to_string(sessionId), utl::LogLevel::INFO);
                    
                    // Create player entity if it doesn't exist
                    if (m_playerEntities.find(sessionId) == m_playerEntities.end())
                    {
                        ecs::Entity playerEntity = m_registry.createEntity()
                                                        .with<ecs::Transform>("player_transform_" + std::to_string(sessionId),
                                                                              200.F + (sessionId % 1000), 100.F, 0.F)
                                                        .with<ecs::Velocity>("player_velocity_" + std::to_string(sessionId), 0.F, 0.F)
                                                        .with<ecs::Player>("player_" + std::to_string(sessionId), true)
                                                        .build();
                        m_playerEntities[sessionId] = playerEntity;
                        utl::Logger::log("RTypeServer: Created player entity for sessionId " + std::to_string(sessionId), utl::LogLevel::INFO);
                    }
                    
                    // Apply input to player entity
                    if (sessionId != 0 && m_playerEntities.find(sessionId) != m_playerEntities.end())
                    {
                        ecs::Entity playerEntity = m_playerEntities[sessionId];
                        auto *velocity = m_registry.getComponent<ecs::Velocity>(playerEntity);
                        auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);
                        
                        if (velocity && transform)
                        {
                            // Calculate velocity based on input
                            const float SPEED = 500.0f;
                            velocity->x = 0.0f;
                            velocity->y = 0.0f;
                            
                            if (up) velocity->y = -SPEED;
                            if (down) velocity->y = SPEED;
                            if (left) velocity->x = -SPEED;
                            if (right) velocity->x = SPEED;
                            
                            // Normalize diagonal movement
                            if (velocity->x != 0.0f && velocity->y != 0.0f)
                            {
                                velocity->x *= 0.707f; // sqrt(2)/2
                                velocity->y *= 0.707f;
                            }
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                utl::Logger::log("RTypeServer: Error processing input: " + std::string(e.what()), utl::LogLevel::WARNING);
            }
        }
    }
    
    utl::Logger::log("RTypeServer: Processed " + std::to_string(events.size()) + " events", utl::LogLevel::INFO);
    
    updateEntities(deltaTime);
    
    // Broadcast at 60 Hz
    if (m_lastBroadcastTime >= BROADCAST_INTERVAL)
    {
        broadcastWorldState();
        m_lastBroadcastTime = 0.0f;
    }
}

void gme::RTypeServer::processInputs()
{
    auto events = m_eventBus.consumeForTarget(utl::GAME_LOGIC); // Component ID = GAME_LOGIC (ID 3)
    
    utl::Logger::log("RTypeServer: Processing " + std::to_string(events.size()) + " events", utl::LogLevel::INFO);
    
    for (const auto &event : events)
    {
        if (event.type == utl::EventType::PLAYER_INPUT_RECEIVED)
        {
            try
            {
                // Input data comes as: [up, down, left, right, shoot]
                if (event.data.size() >= 5)
                {
                    bool up = event.data[0] != 0;
                    bool down = event.data[1] != 0;
                    bool left = event.data[2] != 0;
                    bool right = event.data[3] != 0;
                    bool shoot = event.data[4] != 0;

                    // Extract sessionId from event data or use sourceId
                    std::uint32_t sessionId = event.sourceId;
                    
                    utl::Logger::log("RTypeServer: Received input from sessionId " + std::to_string(sessionId), utl::LogLevel::INFO);
                    
                    // Create player entity if it doesn't exist
                    if (m_playerEntities.find(sessionId) == m_playerEntities.end())
                    {
                        ecs::Entity playerEntity = m_registry.createEntity()
                                                        .with<ecs::Transform>("player_transform_" + std::to_string(sessionId),
                                                                              200.F + (sessionId % 1000), 100.F, 0.F)
                                                        .with<ecs::Velocity>("player_velocity_" + std::to_string(sessionId), 0.F, 0.F)
                                                        .with<ecs::Player>("player_" + std::to_string(sessionId), true)
                                                        .build();
                        m_playerEntities[sessionId] = playerEntity;
                        utl::Logger::log("RTypeServer: Created player entity for sessionId " + std::to_string(sessionId), utl::LogLevel::INFO);
                    }
                    
                    // Apply input to player entity
                    if (sessionId != 0 && m_playerEntities.find(sessionId) != m_playerEntities.end())
                    {
                        ecs::Entity playerEntity = m_playerEntities[sessionId];
                        auto *velocity = m_registry.getComponent<ecs::Velocity>(playerEntity);
                        auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);
                        
                        if (velocity && transform)
                        {
                            // Calculate velocity based on input
                            const float SPEED = 500.0f;
                            const float DIAGONAL_SPEED = SPEED * 0.707f;
                            
                            velocity->x = 0.0f;
                            velocity->y = 0.0f;

                            if (up && right)
                            {
                                velocity->x = DIAGONAL_SPEED;
                                velocity->y = -DIAGONAL_SPEED;
                            }
                            else if (up && left)
                            {
                                velocity->x = -DIAGONAL_SPEED;
                                velocity->y = -DIAGONAL_SPEED;
                            }
                            else if (down && right)
                            {
                                velocity->x = DIAGONAL_SPEED;
                                velocity->y = DIAGONAL_SPEED;
                            }
                            else if (down && left)
                            {
                                velocity->x = -DIAGONAL_SPEED;
                                velocity->y = DIAGONAL_SPEED;
                            }
                            else
                            {
                                if (up) velocity->y = -SPEED;
                                if (down) velocity->y = SPEED;
                                if (left) velocity->x = -SPEED;
                                if (right) velocity->x = SPEED;
                            }

                            // Handle shoot
                            if (shoot)
                            {
                                // TODO: Spawn projectile
                            }
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                utl::Logger::log("RTypeServer: Error processing input: " + std::string(e.what()), utl::LogLevel::WARNING);
            }
        }
    }
}

void gme::RTypeServer::updateEntities(float deltaTime)
{
    // Update all entities based on deltaTime
    auto &players = m_registry.getAll<ecs::Player>();
    auto &transforms = m_registry.getAll<ecs::Transform>();
    auto &velocities = m_registry.getAll<ecs::Velocity>();

    for (auto &[entity, player] : players)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(entity);
        auto *velocity = m_registry.getComponent<ecs::Velocity>(entity);

        if (transform && velocity)
        {
            // Apply velocity to position
            transform->x += velocity->x * deltaTime;
            transform->y += velocity->y * deltaTime;

            // Clamp to window bounds
            transform->x = std::max(0.0f, std::min(transform->x, 1920.0f));
            transform->y = std::max(0.0f, std::min(transform->y, 1080.0f));
        }
    }

    // TODO: Update enemies, projectiles, collisions, etc.
}

void gme::RTypeServer::broadcastWorldState()
{
    try
    {
        rnp::PacketWorldState worldState;
        worldState.serverTick = static_cast<std::uint32_t>(m_lastBroadcastTime * 60.0f);

        // Serialize all player entities
        for (auto &[sessionId, playerEntity] : m_playerEntities)
        {
            auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);
            auto *velocity = m_registry.getComponent<ecs::Velocity>(playerEntity);

            if (transform && velocity)
            {
                rnp::EntityState entityState;
                entityState.id = sessionId; // Use sessionId as entity ID
                entityState.type = static_cast<std::uint16_t>(rnp::EntityType::PLAYER);
                entityState.x = transform->x;
                entityState.y = transform->y;
                entityState.vx = velocity->x;
                entityState.vy = velocity->y;
                entityState.stateFlags = 0;
                
                worldState.entities.push_back(entityState);
            }
        }
        
        // Serialize all projectile entities
        for (auto &[projId, projEntity] : m_projectileEntities)
        {
            auto *transform = m_registry.getComponent<ecs::Transform>(projEntity);
            auto *velocity = m_registry.getComponent<ecs::Velocity>(projEntity);

            if (transform && velocity)
            {
                rnp::EntityState entityState;
                entityState.id = projId;
                entityState.type = static_cast<std::uint16_t>(rnp::EntityType::PROJECTILE);
                entityState.x = transform->x;
                entityState.y = transform->y;
                entityState.vx = velocity->x;
                entityState.vy = velocity->y;
                entityState.stateFlags = 0;
                
                worldState.entities.push_back(entityState);
            }
        }

        worldState.entityCount = static_cast<std::uint16_t>(worldState.entities.size());

        // Create full packet with header for each client
        std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> clientPackets;
        
        for (const auto &[sessionId, playerEntity] : m_playerEntities)
        {
            if (clientPackets.find(sessionId) != clientPackets.end())
                continue;
                
            // Create packet with header for this session
            rnp::Serializer packetSerializer;
            
            // Create header
            rnp::PacketHeader header;
            header.type = static_cast<std::uint8_t>(rnp::PacketType::WORLD_STATE);
            
            // Serialize world state to calculate length
            rnp::Serializer tempSerializer;
            tempSerializer.serializeWorldState(worldState);
            header.length = static_cast<std::uint16_t>(tempSerializer.getData().size());
            header.sessionId = sessionId;
            
            // Serialize header and world state
            packetSerializer.serializeHeader(header);
            packetSerializer.serializeWorldState(worldState);
            
            clientPackets[sessionId] = packetSerializer.getData();
        }
        
        // Broadcast packets
        for (const auto &[sessionId, packet] : clientPackets)
        {
            utl::Event broadcastEvent(utl::EventType::SEND_TO_CLIENT, 1, utl::NETWORK_SERVER);
            std::vector<std::uint8_t> eventData(sizeof(std::uint32_t) + packet.size());
            std::memcpy(eventData.data(), &sessionId, sizeof(std::uint32_t));
            std::memcpy(eventData.data() + sizeof(std::uint32_t), packet.data(), packet.size());
            broadcastEvent.data = eventData;
            m_eventBus.publish(broadcastEvent);
        }
        
        utl::Logger::log("RTypeServer: Broadcasted world state to " + std::to_string(clientPackets.size()) + " clients", utl::LogLevel::INFO);
    }
    catch (const std::exception &e)
    {
        utl::Logger::log("RTypeServer: Error broadcasting world state: " + std::string(e.what()), utl::LogLevel::WARNING);
    }
}