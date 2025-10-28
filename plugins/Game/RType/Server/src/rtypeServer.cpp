#include "RTypeServer/RTypeServer.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "Utils/EventBus.hpp"
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
                        ecs::Entity playerEntity =
                            m_registry.createEntity()
                                .with<ecs::Transform>("player_transform_" + std::to_string(sessionId),
                                                      200.F + (i * 200.F), 100.F, 0.F)
                                .with<ecs::Velocity>("player_velocity_" + std::to_string(sessionId), 0.F, 0.F)
                                .with<ecs::Player>("player_" + std::to_string(sessionId), true)
                                .with<ecs::BeamCharge>("beam_charge_" + std::to_string(sessionId), 0.0f, 1.0f)
                                .with<ecs::Hitbox>("player_hitbox_" + std::to_string(sessionId), 10.0f, 0.0f, 0.0f)
                                .build();
                        m_playerEntities[sessionId] = playerEntity;
                        m_playerShooting[sessionId] = false; // Initialize shooting state
                        utl::Logger::log("RTypeServer: Created player entity for sessionId " +
                                             std::to_string(sessionId) + " at position " +
                                             std::to_string(200.F + (i * 200.F)),
                                         utl::LogLevel::INFO);
                    }
                }
            }
        }
        else if (event.type == utl::EventType::PLAYER_INPUT_RECEIVED)
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

                    std::uint32_t sessionId = event.sourceId;


                    // Create player entity if it doesn't exist
                    if (m_playerEntities.find(sessionId) == m_playerEntities.end())
                    {
                        ecs::Entity playerEntity =
                            m_registry.createEntity()
                                .with<ecs::Transform>("player_transform_" + std::to_string(sessionId),
                                                      200.F + (sessionId % 1000), 100.F, 0.F)
                                .with<ecs::Velocity>("player_velocity_" + std::to_string(sessionId), 0.F, 0.F)
                                .with<ecs::Player>("player_" + std::to_string(sessionId), true)
                                .with<ecs::BeamCharge>("beam_charge_" + std::to_string(sessionId), 0.0f, 1.0f)
                                .with<ecs::Hitbox>("player_hitbox_" + std::to_string(sessionId), 10.0f, 0.0f, 0.0f)
                                .build();
                        m_playerEntities[sessionId] = playerEntity;
                        m_playerShooting[sessionId] = false; // Initialize shooting state
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
                                if (up)
                                    velocity->y = -SPEED;
                                if (down)
                                    velocity->y = SPEED;
                                if (left)
                                    velocity->x = -SPEED;
                                if (right)
                                    velocity->x = SPEED;
                            }

                            // Track shooting state
                            m_playerShooting[sessionId] = shoot;
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                utl::Logger::log("RTypeServer: Error processing input: " + std::string(e.what()),
                                 utl::LogLevel::WARNING);
            }
        }
    }

    // Update cooldown timers
    for (auto &[sessionId, time] : m_lastShotTime)
    {
        time += deltaTime;
    }
    
    // Update beam charges for all players (continuous charging)
    for (auto &[sessionId, playerEntity] : m_playerEntities)
    {
        auto *beamCharge = m_registry.getComponent<ecs::BeamCharge>(playerEntity);
        if (beamCharge)
        {
            // Check if player is currently pressing shoot
            bool isShooting = m_playerShooting.find(sessionId) != m_playerShooting.end() && m_playerShooting[sessionId];
            
            if (isShooting)
            {
                // Continue charging
                const float CHARGE_RATE = 2.0f;
                beamCharge->current_charge += CHARGE_RATE * deltaTime;
                if (beamCharge->current_charge > beamCharge->max_charge)
                    beamCharge->current_charge = beamCharge->max_charge;
            }
            else if (beamCharge->current_charge > 0.01f)
            {
                // Release charge and fire
                auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);
                if (transform)
                {
                    const float PLAYER_SPRITE_WIDTH = 33.0f;
                    const float PLAYER_SPRITE_HEIGHT = 17.0f;
                    const float PLAYER_SCALE = 2.0f;
                    const float PLAYER_WIDTH = PLAYER_SPRITE_WIDTH * PLAYER_SCALE;
                    const float PLAYER_HEIGHT = PLAYER_SPRITE_HEIGHT * PLAYER_SCALE;
                    float projectileX = transform->x + PLAYER_WIDTH + 10.0f;
                    float projectileY = transform->y + PLAYER_HEIGHT / 2.0f;
                    
                    bool isSupercharged = beamCharge->current_charge >= 0.5f;
                    float projectileSpeed = isSupercharged ? 1200.0f : 800.0f;
                    
                    spawnProjectile(sessionId, projectileX, projectileY, projectileSpeed, 0.0f, isSupercharged);
                    beamCharge->current_charge = 0.0f;
                }
            }
        }
    }
    
    updateEntities(deltaTime);

    // Broadcast at 60 Hz
    if (m_lastBroadcastTime >= BROADCAST_INTERVAL)
    {
        broadcastWorldState();
        m_lastBroadcastTime = 0.0f;
    }
}

void gme::RTypeServer::updateEntities(float deltaTime)
{
    auto &players = m_registry.getAll<ecs::Player>();

    for (auto &[entity, player] : players)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(entity);
        auto *velocity = m_registry.getComponent<ecs::Velocity>(entity);
        auto *hitbox = m_registry.getComponent<ecs::Hitbox>(entity);

        if (transform && velocity)
        {
            transform->x += velocity->x * deltaTime;
            transform->y += velocity->y * deltaTime;

            if (hitbox)
            {
                const float PLAYER_SPRITE_WIDTH = 33.0f;
                const float PLAYER_SPRITE_HEIGHT = 17.0f;
                const float PLAYER_SCALE = 2.0f;
                const float PLAYER_WIDTH = PLAYER_SPRITE_WIDTH * PLAYER_SCALE;
                const float PLAYER_HEIGHT = PLAYER_SPRITE_HEIGHT * PLAYER_SCALE;
                const float PLAYER_HITBOX_RADIUS = hitbox->radius;

                float minX = PLAYER_HITBOX_RADIUS;
                float maxX = 1920.0f - (PLAYER_WIDTH - PLAYER_HITBOX_RADIUS);
                float minY = PLAYER_HITBOX_RADIUS;
                float maxY = 1080.0f - (PLAYER_HEIGHT - PLAYER_HITBOX_RADIUS);

                transform->x = std::max(minX, std::min(transform->x, maxX));
                transform->y = std::max(minY, std::min(transform->y, maxY));
            }
            else
            {
                transform->x = std::max(0.0f, std::min(transform->x, 1920.0f));
                transform->y = std::max(0.0f, std::min(transform->y, 1080.0f));
            }
        }
    }

    // Update projectiles
    for (auto it = m_projectileEntities.begin(); it != m_projectileEntities.end();)
    {
        auto &[projId, projEntity] = *it;
        auto *transform = m_registry.getComponent<ecs::Transform>(projEntity);
        auto *velocity = m_registry.getComponent<ecs::Velocity>(projEntity);

        if (transform && velocity)
        {
            transform->x += velocity->x * deltaTime;
            transform->y += velocity->y * deltaTime;

            // Remove projectiles that are off-screen
            if (transform->x > 2000.0f || transform->x < -100.0f)
            {
                m_registry.removeComponent<ecs::Transform>(projEntity);
                m_registry.removeComponent<ecs::Velocity>(projEntity);
                it = m_projectileEntities.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
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
            auto *beamCharge = m_registry.getComponent<ecs::BeamCharge>(playerEntity);

            if (transform && velocity)
            {
                rnp::EntityState entityState;
                entityState.id = sessionId; // Use sessionId as entity ID
                entityState.type = static_cast<std::uint16_t>(rnp::EntityType::PLAYER);
                entityState.x = transform->x;
                entityState.y = transform->y;
                entityState.vx = velocity->x;
                entityState.vy = velocity->y;
                
                // Encode beam charge in stateFlags (0-255 for 0.0-1.0)
                std::uint8_t chargeLevel = 0;
                if (beamCharge)
                {
                    chargeLevel = static_cast<std::uint8_t>(beamCharge->current_charge * 255.0f);
                }
                entityState.stateFlags = chargeLevel;

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
    }
    catch (const std::exception &e)
    {
        utl::Logger::log("RTypeServer: Error broadcasting world state: " + std::string(e.what()),
                         utl::LogLevel::WARNING);
    }
}

void gme::RTypeServer::spawnProjectile(std::uint32_t playerId, float x, float y, float vx, float vy, bool isSupercharged)
{
    std::uint32_t projectileId = m_nextProjectileId++;
    
    ecs::Entity projectile = m_registry.createEntity()
                                 .with<ecs::Transform>("projectile_transform_" + std::to_string(projectileId), x, y, 0.F)
                                 .with<ecs::Velocity>("projectile_velocity_" + std::to_string(projectileId), vx, vy)
                                 .build();
    
    m_projectileEntities[projectileId] = projectile;
}
