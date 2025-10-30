///
/// @file rtypeServer.cpp
/// @brief Implementation of RTypeServer with new entity management system
/// @namespace gme
///

#include "RTypeServer/RTypeServer.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "Utils/EventBus.hpp"
#include "Utils/Logger.hpp"
#include <algorithm>
#include <ranges>

namespace gme
{
    RTypeServer::RTypeServer() : m_eventBus(utl::EventBus::getInstance())
    {
        // Register this component with the event bus
        m_eventBus.registerComponent(utl::GAME_LOGIC, "RTypeServer");

        // Subscribe to events
        m_eventBus.subscribe(utl::GAME_LOGIC, utl::EventType::SERVER_START);
        m_eventBus.subscribe(utl::GAME_LOGIC, utl::EventType::PLAYER_INPUT_RECEIVED);

        // Initialize entity manager
        m_entityManager = std::make_unique<EntityManager>(m_registry);

        // Initialize game systems
        m_collisionSystem = std::make_unique<CollisionSystem>(m_registry, *m_entityManager);
        m_enemyAISystem = std::make_unique<EnemyAISystem>(m_registry, *m_entityManager);
        m_enemySpawnSystem = std::make_unique<EnemySpawnSystem>(*m_entityManager);

        utl::Logger::log("RTypeServer: Initialized with entity management system", utl::LogLevel::INFO);
    }

    void RTypeServer::start()
    {
        m_gameState = State::PLAYING;
        m_levelState = LevelState::WAITING_FOR_PLAYERS;

        // Enable enemy spawning
        m_enemySpawnSystem->setEnabled(true);

        utl::Logger::log("RTypeServer: Start called - enemy spawning enabled", utl::LogLevel::INFO);
    }

    void RTypeServer::stop()
    {
        m_gameState = State::LOSE;

        // Clear all entities
        m_entityManager->clear();

        // Reset systems
        m_enemySpawnSystem->reset();

        // Clear player tracking
        m_playerShooting.clear();
        m_lastShotTime.clear();

        utl::Logger::log("RTypeServer: Stopped", utl::LogLevel::INFO);
    }

    void RTypeServer::update(const float deltaTime)
    {
        m_lastBroadcastTime += deltaTime;

        auto events = m_eventBus.consumeForTarget(utl::GAME_LOGIC);
        for (const auto &event : events)
        {
            if (event.type == utl::EventType::SERVER_START)
            {
                processServerStartEvent(event);
            }
            else if (event.type == utl::EventType::PLAYER_INPUT_RECEIVED)
            {
                processPlayerInputEvent(event);
            }
        }

        // Update all game systems
        updateSystems(deltaTime);

        // Update entity physics
        updateEntities(deltaTime);

        // Update entity lifetimes (for projectiles with limited lifetime)
        m_entityManager->updateLifetimes(deltaTime);

        // Clean up destroyed entities
        m_entityManager->cleanupDestroyedEntities();

        // Broadcast world state at 60 Hz
        if (m_lastBroadcastTime >= BROADCAST_INTERVAL)
        {
            broadcastWorldState();
            m_lastBroadcastTime = 0.0f;
        }
    }

    void RTypeServer::processServerStartEvent(const utl::Event &event)
    {
        // Create player entities from sessionIds
        if (event.data.size() >= sizeof(std::uint32_t))
        {
            size_t playerCount = event.data.size() / sizeof(std::uint32_t);

            for (size_t i = 0; i < playerCount; ++i)
            {
                std::uint32_t sessionId;
                std::memcpy(&sessionId, event.data.data() + i * sizeof(std::uint32_t), sizeof(std::uint32_t));

                if (!m_entityManager->hasPlayer(sessionId))
                {
                    float startX = 200.0f + (i * 200.0f);
                    float startY = 100.0f + (i * 100.0f);

                    m_entityManager->createPlayer(sessionId, startX, startY);
                    m_playerShooting[sessionId] = false;
                    m_lastShotTime[sessionId] = 0.0f;

                    utl::Logger::log("RTypeServer: Created player entity for sessionId " + std::to_string(sessionId),
                                     utl::LogLevel::INFO);
                }
            }

            // Start enemy spawning when game starts
            if (m_levelState == LevelState::WAITING_FOR_PLAYERS)
            {
                m_levelState = LevelState::IN_PROGRESS;
                m_enemySpawnSystem->setEnabled(true);
                utl::Logger::log("RTypeServer: Game started, enemy spawning enabled", utl::LogLevel::INFO);
            }
        }
    }
    for (auto &time : m_lastShotTime | std::views::values)
    {
        time += deltaTime;
    }
    for (auto &[sessionId, playerEntity] : m_playerEntities)
    {
        if (auto *beamCharge = m_registry.getComponent<ecs::BeamCharge>(playerEntity); beamCharge != nullptr)
        {
            if (m_playerShooting.contains(sessionId) && m_playerShooting[sessionId])
            {
                beamCharge->current_charge += 2.0F * deltaTime;
                beamCharge->current_charge = std::min(beamCharge->current_charge, beamCharge->max_charge);
            }
            else if (beamCharge->current_charge > 0.01f)
            {
                if (const auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity))
                {
                    const float projectileX = transform->x + utl::GameConfig::Player::WIDTH + 10.0F;
                    const float projectileY = transform->y + (utl::GameConfig::Player::HEIGHT / 2.0f);
                    const bool isSupercharged = beamCharge->current_charge >= 0.5f;
                    const float projectileSpeed = beamCharge->current_charge >= 0.5f ? 1200.0f : 800.0f;
                    spawnProjectile(sessionId, projectileX, projectileY, projectileSpeed, 0.0f, isSupercharged);
                    beamCharge->current_charge = 0.0f;
                }
            }
        }
    }

    void RTypeServer::processPlayerInputEvent(const utl::Event &event)
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
                if (!m_entityManager->hasPlayer(sessionId))
                {
                    // Spread players vertically to avoid spawn overlap
                    float spawnX = 200.0f;
                    float spawnY = 200.0f + ((sessionId % 4) * 200.0f);
                    m_entityManager->createPlayer(sessionId, spawnX, spawnY);
                    m_playerShooting[sessionId] = false;
                    m_lastShotTime[sessionId] = 0.0f;
                }

                // Apply input to player entity
                if (sessionId != 0 && m_entityManager->hasPlayer(sessionId))
                {
                    ecs::Entity playerEntity = m_entityManager->getPlayer(sessionId);
                    auto *velocity = m_registry.getComponent<ecs::Velocity>(playerEntity);
                    auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);

                    if (velocity && transform)
                    {
                        // Calculate velocity based on input
                        const float SPEED = 500.0f;
                        const float DIAGONAL_SPEED = SPEED * 0.707f;

                        velocity->x = 0.0f;
                        velocity->y = 0.0f;

                        // Handle diagonal movement
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
                            // Handle cardinal directions
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
            utl::Logger::log("RTypeServer: Error processing input: " + std::string(e.what()), utl::LogLevel::WARNING);
        }
    }

    void RTypeServer::updateSystems(float deltaTime)
    {
        // Update all game systems in order

        m_enemyAISystem->update(m_registry, deltaTime);

        m_enemySpawnSystem->update(m_registry, deltaTime);

        for (auto &[sessionId, _] : m_entityManager->getPlayers())
        {
            handlePlayerShooting(sessionId, deltaTime);
        }

        m_collisionSystem->update(m_registry, deltaTime);
    }

    void RTypeServer::handlePlayerShooting(std::uint32_t sessionId, float deltaTime)
    {
        if (m_lastShotTime.find(sessionId) == m_lastShotTime.end())
        {
            m_lastShotTime[sessionId] = 0.0f;
        }

        m_lastShotTime[sessionId] += deltaTime;

        ecs::Entity playerEntity = m_entityManager->getPlayer(sessionId);
        if (playerEntity == ecs::INVALID_ENTITY)
            return;

        auto *beamCharge = m_registry.getComponent<ecs::BeamCharge>(playerEntity);
        auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);

        if (!beamCharge || !transform)
            return;

        bool isShooting = m_playerShooting.find(sessionId) != m_playerShooting.end() && m_playerShooting[sessionId];

        if (isShooting)
        {
            const float CHARGE_RATE = 2.0f;
            beamCharge->current_charge += CHARGE_RATE * deltaTime;
            if (beamCharge->current_charge > beamCharge->max_charge)
                beamCharge->current_charge = beamCharge->max_charge;
        }
        else if (beamCharge->current_charge > 0.01f)
        {
            if (m_lastShotTime[sessionId] >= PROJECTILE_COOLDOWN)
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

                m_entityManager->createPlayerProjectile(sessionId, projectileX, projectileY, projectileSpeed, 0.0f,
                                                        isSupercharged);

                beamCharge->current_charge = 0.0f;
                m_lastShotTime[sessionId] = 0.0f;
            }
        }
    }

    void RTypeServer::updateEntities(float deltaTime)
    {
        const auto &players = m_entityManager->getPlayers();
        for (const auto &[sessionId, entity] : players)
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

        // Update enemy positions
        const auto &enemies = m_entityManager->getEnemies();
        for (const auto &[enemyId, entity] : enemies)
        {
            // Skip inactive entities
            const auto *metadata = m_entityManager->getEntityMetadata(enemyId);
            if (!metadata || !metadata->isActive)
                continue;

            auto *transform = m_registry.getComponent<ecs::Transform>(entity);
            auto *velocity = m_registry.getComponent<ecs::Velocity>(entity);

            if (transform && velocity)
            {
                transform->x += velocity->x * deltaTime;
                transform->y += velocity->y * deltaTime;

                const float MARGIN = 200.0f;
                if (transform->x < -MARGIN || transform->x > 1920.0f + MARGIN || transform->y < -MARGIN ||
                    transform->y > 1080.0f + MARGIN)
                {
                    m_entityManager->destroyEnemy(enemyId);
                }
            }
        }

        const auto &projectiles = m_entityManager->getProjectiles();
        for (const auto &[projectileId, entity] : projectiles)
        {
            const auto *metadata = m_entityManager->getEntityMetadata(projectileId);
            if (!metadata || !metadata->isActive)
                continue;

            auto *transform = m_registry.getComponent<ecs::Transform>(entity);
            auto *velocity = m_registry.getComponent<ecs::Velocity>(entity);

            if (transform && velocity)
            {
                transform->x += velocity->x * deltaTime;
                transform->y += velocity->y * deltaTime;

                // Remove projectiles that are off-screen
                if (transform->x > 2100.0f || transform->x < -100.0f || transform->y > 1180.0f ||
                    transform->y < -100.0f)
                {
                    m_entityManager->destroyProjectile(projectileId);
                }
            }
        }
    }

    void RTypeServer::broadcastWorldState()
    {
        try
        {
            rnp::PacketWorldState worldState;
            worldState.serverTick = static_cast<std::uint32_t>(m_lastBroadcastTime * 60.0f);

            // Get all entity states from EntityManager
            worldState.entities = m_entityManager->getAllEntityStates();
            worldState.entityCount = static_cast<std::uint16_t>(worldState.entities.size());

            // Create full packet with header for each client
            const auto &players = m_entityManager->getPlayers();

            for (const auto &[sessionId, playerEntity] : players)
            {
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

                std::vector<std::uint8_t> packet = packetSerializer.getData();

                // Send packet via EventBus
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

} // namespace gme
