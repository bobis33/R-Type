///
/// @file rtypeServer.cpp
/// @brief Implementation of RTypeServer with new entity management system
/// @namespace gme
///

#include <algorithm>
#include <ranges>

#include "ECS/Component.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "RTypeServer/RTypeServer.hpp"
#include "Utils/Common.hpp"
#include "Utils/EventBus.hpp"
#include "Utils/Logger.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

namespace gme
{
    RTypeServer::RTypeServer() : m_eventBus(utl::EventBus::getInstance())
    {
        // Register this component with the event bus
        m_eventBus.registerComponent(utl::GAME_LOGIC, "RTypeServer");

        // Subscribe to events
        m_eventBus.subscribe(utl::GAME_LOGIC, utl::EventType::SERVER_START);
        m_eventBus.subscribe(utl::GAME_LOGIC, utl::EventType::PLAYER_INPUT_RECEIVED);
        m_eventBus.subscribe(utl::GAME_LOGIC, utl::EventType::GAME_START);

        // Initialize entity manager
        m_entityManager = std::make_unique<EntityManager>(m_registry);

        // Initialize game systems
        m_collisionSystem = std::make_unique<CollisionSystem>(m_registry, *m_entityManager);
        m_enemyAISystem = std::make_unique<EnemyAISystem>(m_registry, *m_entityManager);
        m_enemySpawnSystem = std::make_unique<EnemySpawnSystem>(*m_entityManager);

        // Initialize wave manager with default waves
        m_waveManager = std::make_unique<WaveManager>(*m_entityManager);
        m_waveManager->setupDefaultWaves();

        utl::Logger::log("RTypeServer: Initialized with entity management system", utl::LogLevel::INFO);
    }

    void RTypeServer::start()
    {
        m_gameState = State::PLAYING;
        m_levelState = LevelState::WAITING_FOR_PLAYERS;

        // Disable old enemy spawning system, use wave manager instead
        m_enemySpawnSystem->setEnabled(false);

        // DON'T start wave manager here - wait for GAME_START event from waiting room
        utl::Logger::log("RTypeServer: Server started - waiting for game to begin", utl::LogLevel::INFO);
    }

    void RTypeServer::stop()
    {
        m_gameState = State::LOSE;

        // Clear all entities
        m_entityManager->clear();

        // Reset systems
        m_enemySpawnSystem->reset();
        m_waveManager->reset();

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
            else if (event.type == utl::EventType::GAME_START)
            {
                processGameStartEvent(event);
            }
            else if (event.type == utl::EventType::PLAYER_DISCONNECTED)
            {
                processPlayerDisconnectEvent(event);
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

        // Check for game over conditions
        checkGameOver();

        // Broadcast world state at 60 Hz
        if (m_lastBroadcastTime >= utl::GameConfig::Server::Network::BROADCAST_INTERVAL)
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
                        constexpr float SPEED = 500.0f;
                        constexpr float DIAGONAL_SPEED = SPEED * 0.707f;

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

    void RTypeServer::processGameStartEvent(const utl::Event &event)
    {
        (void)event; // Event data contains lobby info if needed

        utl::Logger::log("RTypeServer: GAME_START event received - starting waves!", utl::LogLevel::INFO);

        // Start the wave manager now that the game is actually starting
        if (!m_waveManager->isActive())
        {
            m_waveManager->start();
            m_levelState = LevelState::IN_PROGRESS;
            utl::Logger::log("RTypeServer: Wave system started - 3 waves of 30 seconds", utl::LogLevel::INFO);
        }
        else
        {
            utl::Logger::log("RTypeServer: Wave system already active", utl::LogLevel::WARNING);
        }
    }

    void RTypeServer::updateSystems(float deltaTime)
    {
        // Update all game systems in order

        m_enemyAISystem->update(m_registry, deltaTime);

        // Update wave manager instead of simple enemy spawning
        // Assuming 1920 as default screen width (can be made configurable later)
        m_waveManager->update(m_registry, deltaTime, 1920);

        for (const auto &sessionId : m_entityManager->getPlayers() | std::views::keys)
        {
            handlePlayerShooting(sessionId, deltaTime);
        }

        m_collisionSystem->update(m_registry, deltaTime);
    }

    void RTypeServer::checkGameOver()
    {
        // Don't check if game is not in progress
        if (m_levelState != LevelState::IN_PROGRESS)
        {
            return;
        }

        // Check if there are no more players at all (all disconnected)
        const std::uint32_t totalPlayerCount = static_cast<std::uint32_t>(m_entityManager->getPlayers().size());
        if (totalPlayerCount == 0)
        {
            utl::Logger::log("RTypeServer: No players left in game - resetting", utl::LogLevel::INFO);
            m_levelState = LevelState::WAITING_FOR_PLAYERS;
            m_entityManager->clear();
            if (m_waveManager)
            {
                m_waveManager->reset();
            }
            return;
        }

        // Check if all waves are completed
        const bool allWavesComplete = m_waveManager->isCompleted();

        // Check alive player count
        const std::uint32_t alivePlayerCount = m_entityManager->getAlivePlayerCount();

        // Game Over conditions:
        // 1. Only 1 or 0 players alive (in multiplayer context)
        // 2. All waves completed (victory)
        bool shouldGameOver = false;
        std::string reason;

        if (alivePlayerCount == 0)
        {
            shouldGameOver = true;
            reason = "All players died";
            m_levelState = LevelState::LOOSE;
        }
        else if (totalPlayerCount > 1 && alivePlayerCount == 1)
        {
            shouldGameOver = true;
            reason = "Only one player remaining";
            m_levelState = LevelState::LOOSE;
        }
        else if (allWavesComplete)
        {
            shouldGameOver = true;
            reason = "All waves completed";
            m_levelState = LevelState::COMPLETED;
        }

        if (shouldGameOver)
        {
            utl::Logger::log("RTypeServer: Game Over - " + reason, utl::LogLevel::INFO);

            // Broadcast GAME_OVER event to all clients
            std::vector<std::uint8_t> gameOverData;
            gameOverData.resize(1);
            gameOverData[0] = static_cast<std::uint8_t>(m_levelState);

            m_eventBus.publish(utl::EventType::GAME_OVER, gameOverData, utl::GAME_LOGIC, 0);

            // Reset game state for next round
            m_levelState = LevelState::WAITING_FOR_PLAYERS;
        }
    }

    void RTypeServer::processPlayerDisconnectEvent(const utl::Event &event)
    {
        std::uint32_t sessionId = event.sourceId;

        utl::Logger::log("RTypeServer: Player " + std::to_string(sessionId) + " disconnected", utl::LogLevel::INFO);

        // Remove player from entity manager
        if (m_entityManager->hasPlayer(sessionId))
        {
            m_entityManager->destroyPlayer(sessionId);
            utl::Logger::log("RTypeServer: Removed player " + std::to_string(sessionId) + " from game",
                             utl::LogLevel::INFO);
        }

        // Check if there are no more players
        if (m_entityManager->getPlayers().empty())
        {
            utl::Logger::log("RTypeServer: No more players in game - resetting game state", utl::LogLevel::INFO);
            m_levelState = LevelState::WAITING_FOR_PLAYERS;

            // Clear all game entities
            m_entityManager->clear();

            // Reset wave manager
            if (m_waveManager)
            {
                m_waveManager->reset();
            }
        }
        else
        {
            // Check if game should end with remaining players
            checkGameOver();
        }
    }

    void RTypeServer::handlePlayerShooting(std::uint32_t sessionId, float deltaTime)
    {
        if (!m_lastShotTime.contains(sessionId))
        {
            m_lastShotTime[sessionId] = 0.0f;
        }

        m_lastShotTime[sessionId] += deltaTime;

        const ecs::Entity playerEntity = m_entityManager->getPlayer(sessionId);
        if (playerEntity == ecs::INVALID_ENTITY)
        {
            return;
        }

        auto *beamCharge = m_registry.getComponent<ecs::BeamCharge>(playerEntity);
        auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);

        if (!beamCharge || !transform)
        {
            return;
        }
        if (m_playerShooting.contains(sessionId) && m_playerShooting[sessionId])
        {
            constexpr float CHARGE_RATE = 2.0f;
            beamCharge->current_charge += CHARGE_RATE * deltaTime;
            if (beamCharge->current_charge > beamCharge->max_charge)
                beamCharge->current_charge = beamCharge->max_charge;
        }
        else if (beamCharge->current_charge > 0.01f)
        {
            if (m_lastShotTime[sessionId] >= PROJECTILE_COOLDOWN)
            {
                constexpr float PLAYER_WIDTH = utl::GameConfig::Player::SPRITE_WIDTH * utl::GameConfig::Player::SCALE;
                constexpr float PLAYER_HEIGHT = utl::GameConfig::Player::SPRITE_HEIGHT * utl::GameConfig::Player::SCALE;

                const float projectileX = transform->x + PLAYER_WIDTH + 10.0f;
                const float projectileY = transform->y + PLAYER_HEIGHT / 2.0f;

                const bool isSupercharged = beamCharge->current_charge >= 0.5f;
                const float projectileSpeed = isSupercharged ? 1200.0f : 800.0f;

                m_entityManager->createPlayerProjectile(sessionId, projectileX, projectileY, projectileSpeed, 0.0f,
                                                        isSupercharged);

                beamCharge->current_charge = 0.0f;
                m_lastShotTime[sessionId] = 0.0f;
            }
        }
    }

    void RTypeServer::updateEntities(float deltaTime)
    {
        for (const auto &players = m_entityManager->getPlayers(); const auto &entity : players | std::views::values)
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
                    constexpr float PLAYER_WIDTH =
                        utl::GameConfig::Player::SPRITE_WIDTH * utl::GameConfig::Player::SCALE;
                    constexpr float PLAYER_HEIGHT =
                        utl::GameConfig::Player::SPRITE_HEIGHT * utl::GameConfig::Player::SCALE;
                    const float hitboxRadius = hitbox->radius;

                    float maxX = utl::GameConfig::Server::SCREEN_WIDTH - (PLAYER_WIDTH - hitboxRadius);
                    float maxY = utl::GameConfig::Server::SCREEN_HEIGHT - (PLAYER_HEIGHT - hitboxRadius);

                    transform->x = std::max(hitboxRadius, std::min(transform->x, maxX));
                    transform->y = std::max(hitboxRadius, std::min(transform->y, maxY));
                }
                else
                {
                    transform->x = std::max(0.0f, std::min(transform->x, utl::GameConfig::Server::SCREEN_WIDTH));
                    transform->y = std::max(0.0f, std::min(transform->y, utl::GameConfig::Server::SCREEN_HEIGHT));
                }
            }
        }

        // Update enemy positions
        for (const auto &enemies = m_entityManager->getEnemies(); const auto &[enemyId, entity] : enemies)
        {
            // Skip inactive entities
            if (const auto *metadata = m_entityManager->getEntityMetadata(enemyId); !metadata || !metadata->isActive)
            {
                continue;
            }

            auto *transform = m_registry.getComponent<ecs::Transform>(entity);
            auto *velocity = m_registry.getComponent<ecs::Velocity>(entity);

            if (transform && velocity)
            {
                transform->x += velocity->x * deltaTime;
                transform->y += velocity->y * deltaTime;

                // Destroy enemies that are too far off-screen
                if (transform->x < -utl::GameConfig::Server::WORLD_MARGIN ||
                    transform->x > utl::GameConfig::Server::SCREEN_WIDTH + utl::GameConfig::Server::WORLD_MARGIN ||
                    transform->y < -utl::GameConfig::Server::WORLD_MARGIN ||
                    transform->y > utl::GameConfig::Server::SCREEN_HEIGHT + utl::GameConfig::Server::WORLD_MARGIN)
                {
                    m_entityManager->destroyEnemy(enemyId);
                }
            }
        }

        const auto &projectiles = m_entityManager->getProjectiles();
        for (const auto &[projectileId, entity] : projectiles)
        {
            if (const auto *metadata = m_entityManager->getEntityMetadata(projectileId);
                !metadata || !metadata->isActive)
            {
                continue;
            }

            auto *transform = m_registry.getComponent<ecs::Transform>(entity);
            auto *velocity = m_registry.getComponent<ecs::Velocity>(entity);

            if (transform && velocity)
            {
                transform->x += velocity->x * deltaTime;
                transform->y += velocity->y * deltaTime;

                // Remove projectiles that are off-screen (with margin)
                if (transform->x > utl::GameConfig::Server::SCREEN_WIDTH + utl::GameConfig::Server::WORLD_MARGIN ||
                    transform->x < -utl::GameConfig::Server::WORLD_MARGIN ||
                    transform->y > utl::GameConfig::Server::SCREEN_HEIGHT + utl::GameConfig::Server::WORLD_MARGIN ||
                    transform->y < -utl::GameConfig::Server::WORLD_MARGIN)
                {
                    m_entityManager->destroyProjectile(projectileId);
                }
            }
        }
    }

    void RTypeServer::broadcastWorldState() const
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

            for (const auto &sessionId : players | std::views::keys)
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
