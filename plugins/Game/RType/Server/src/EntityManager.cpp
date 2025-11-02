///
/// @file EntityManager.cpp
/// @brief Implementation of EntityManager for RTypeServer
/// @namespace gme
///

#include "RTypeServer/EntityManager.hpp"
#include "Utils/Logger.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"
#include <algorithm>

namespace gme
{
    EntityManager::EntityManager(ecs::Registry &registry) : m_registry(registry) {}

    ecs::Entity EntityManager::createPlayer(std::uint32_t sessionId, float x, float y)
    {
        if (hasPlayer(sessionId))
        {
            utl::Logger::log("EntityManager: Player with sessionId " + std::to_string(sessionId) + " already exists",
                             utl::LogLevel::WARNING);
            return m_playerEntities[sessionId];
        }

        // Validate spawn position
        x = std::max(0.0f, std::min(x, utl::GameConfig::Server::SCREEN_WIDTH));
        y = std::max(0.0f, std::min(y, utl::GameConfig::Server::SCREEN_HEIGHT));

        ecs::Entity playerEntity =
            m_registry.createEntity()
                .with<ecs::Transform>("player_transform_" + std::to_string(sessionId), x, y, 0.0f)
                .with<ecs::Velocity>("player_velocity_" + std::to_string(sessionId), 0.0f, 0.0f)
                .with<ecs::Player>("player_" + std::to_string(sessionId), true)
                .with<ecs::Health>("player_health_" + std::to_string(sessionId), 100.0f, 100.0f)
                .with<ecs::BeamCharge>("beam_charge_" + std::to_string(sessionId), 0.0f, 1.0f)
                .with<ecs::Hitbox>("player_hitbox_" + std::to_string(sessionId), 10.0f, 0.0f, 0.0f)
                .build();

        m_playerEntities[sessionId] = playerEntity;
        registerEntity(playerEntity, ServerEntityType::PLAYER, sessionId, -1.0f);

        // Initialize score
        m_playerScores[sessionId] = 0;

        utl::Logger::log("EntityManager: Created player entity for sessionId " + std::to_string(sessionId) +
                             " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")",
                         utl::LogLevel::INFO);

        return playerEntity;
    }

    void EntityManager::destroyPlayer(std::uint32_t sessionId)
    {
        // Mark player as inactive so it won't be included in next world state broadcast
        auto *metadata = getEntityMetadata(sessionId);
        if (metadata)
        {
            metadata->isActive = false;
        }

        // Add to destroy queue for cleanup
        m_destroyQueue.push_back(sessionId);

        utl::Logger::log("EntityManager: Marked player " + std::to_string(sessionId) + " for destruction",
                         utl::LogLevel::INFO);
    }

    ecs::Entity EntityManager::getPlayer(std::uint32_t sessionId)
    {
        auto it = m_playerEntities.find(sessionId);
        return (it != m_playerEntities.end()) ? it->second : ecs::INVALID_ENTITY;
    }

    bool EntityManager::hasPlayer(std::uint32_t sessionId) const
    {
        return m_playerEntities.find(sessionId) != m_playerEntities.end();
    }

    ecs::Entity EntityManager::getPlayerEntity(std::uint32_t sessionId) const
    {
        auto it = m_playerEntities.find(sessionId);
        return (it != m_playerEntities.end()) ? it->second : ecs::INVALID_ENTITY;
    }

    void EntityManager::markPlayerAsDead(std::uint32_t sessionId)
    {
        m_deadPlayers.insert(sessionId);
        utl::Logger::log("EntityManager: Player " + std::to_string(sessionId) + " marked as dead", utl::LogLevel::INFO);
    }

    std::uint32_t EntityManager::getAlivePlayerCount() const
    {
        std::uint32_t aliveCount = 0;
        for (const auto &[sessionId, entity] : m_playerEntities)
        {
            if (m_deadPlayers.find(sessionId) == m_deadPlayers.end())
            {
                aliveCount++;
            }
        }
        return aliveCount;
    }

    // ========== Score Management ==========

    void EntityManager::addScore(std::uint32_t sessionId, int points)
    {
        m_playerScores[sessionId] += points;
        utl::Logger::log("EntityManager: Player " + std::to_string(sessionId) + " earned " + std::to_string(points) +
                             " points. Total: " + std::to_string(m_playerScores[sessionId]),
                         utl::LogLevel::INFO);
    }

    int EntityManager::getScore(std::uint32_t sessionId) const
    {
        auto it = m_playerScores.find(sessionId);
        return (it != m_playerScores.end()) ? it->second : 0;
    }

    void EntityManager::resetScore(std::uint32_t sessionId)
    {
        m_playerScores[sessionId] = 0;
        utl::Logger::log("EntityManager: Reset score for player " + std::to_string(sessionId), utl::LogLevel::INFO);
    }

    ecs::Entity EntityManager::createBasicEnemy(float x, float y, float health)
    {
        // Validate parameters
        if (health <= 0.0f)
            health = 50.0f;

        std::uint32_t enemyId = m_nextEnemyId++;

        ecs::Entity enemyEntity =
            m_registry.createEntity()
                .with<ecs::Transform>("enemy_transform_" + std::to_string(enemyId), x, y, 0.0f)
                .with<ecs::Velocity>("enemy_velocity_" + std::to_string(enemyId), -200.0f, 0.0f) // Move left
                .with<ecs::Enemy>("enemy_" + std::to_string(enemyId), health, health, 10.0f, 200.0f, 0.0f, 2.0f)
                .with<ecs::Hitbox>("enemy_hitbox_" + std::to_string(enemyId), 30.0f, 0.0f,
                                   0.0f) // Increased from 15 to 30 for better collision
                .build();

        m_enemyEntities[enemyId] = enemyEntity;
        registerEntity(enemyEntity, ServerEntityType::ENEMY_BASIC, enemyId, -1.0f);

        utl::Logger::log("EntityManager: Created basic enemy " + std::to_string(enemyId) + " at position (" +
                             std::to_string(x) + ", " + std::to_string(y) + ")",
                         utl::LogLevel::INFO);

        return enemyEntity;
    }

    ecs::Entity EntityManager::createAdvancedEnemy(float x, float y, float health)
    {
        // Validate parameters
        if (health <= 0.0f)
            health = 100.0f;

        std::uint32_t enemyId = m_nextEnemyId++;

        ecs::Entity enemyEntity =
            m_registry.createEntity()
                .with<ecs::Transform>("enemy_transform_" + std::to_string(enemyId), x, y, 0.0f)
                .with<ecs::Velocity>("enemy_velocity_" + std::to_string(enemyId), -150.0f, 0.0f)
                .with<ecs::Enemy>("enemy_" + std::to_string(enemyId), health, health, 20.0f, 150.0f, 0.0f, 1.5f)
                .with<ecs::Hitbox>("enemy_hitbox_" + std::to_string(enemyId), 35.0f, 0.0f,
                                   0.0f) // Increased from 20 to 35 for better collision
                .build();

        m_enemyEntities[enemyId] = enemyEntity;
        registerEntity(enemyEntity, ServerEntityType::ENEMY_ADVANCED, enemyId, -1.0f);

        utl::Logger::log("EntityManager: Created advanced enemy " + std::to_string(enemyId) + " at position (" +
                             std::to_string(x) + ", " + std::to_string(y) + ")",
                         utl::LogLevel::INFO);

        return enemyEntity;
    }

    ecs::Entity EntityManager::createBoss(float x, float y, float health)
    {
        // Validate parameters
        if (health <= 0.0f)
            health = 1000.0f;

        std::uint32_t enemyId = m_nextEnemyId++;

        ecs::Entity bossEntity =
            m_registry.createEntity()
                .with<ecs::Transform>("boss_transform_" + std::to_string(enemyId), x, y, 0.0f)
                .with<ecs::Velocity>("boss_velocity_" + std::to_string(enemyId), -50.0f, 0.0f)
                .with<ecs::Enemy>("boss_" + std::to_string(enemyId), health, health, 50.0f, 50.0f, 0.0f, 0.5f)
                .with<ecs::Hitbox>("boss_hitbox_" + std::to_string(enemyId), 90.0f, 0.0f,
                                   0.0f) // Increased from 50 to 90 for better collision with large sprite
                .build();

        m_enemyEntities[enemyId] = bossEntity;
        registerEntity(bossEntity, ServerEntityType::BOSS, enemyId, -1.0f);

        utl::Logger::log("EntityManager: Created boss " + std::to_string(enemyId) + " at position (" +
                             std::to_string(x) + ", " + std::to_string(y) + ")",
                         utl::LogLevel::INFO);

        return bossEntity;
    }

    void EntityManager::destroyEnemy(std::uint32_t enemyId)
    {
        auto *metadata = getEntityMetadata(enemyId);
        if (metadata)
        {
            metadata->isActive = false;
        }
        m_destroyQueue.push_back(enemyId);
    }

    ecs::Entity EntityManager::getEnemy(std::uint32_t enemyId)
    {
        auto it = m_enemyEntities.find(enemyId);
        return (it != m_enemyEntities.end()) ? it->second : ecs::INVALID_ENTITY;
    }

    ecs::Entity EntityManager::createPlayerProjectile(std::uint32_t playerId, float x, float y, float vx, float vy,
                                                      bool isSupercharged)
    {
        std::uint32_t projectileId = m_nextProjectileId++;

        ecs::Projectile::Type projType = isSupercharged ? ecs::Projectile::SUPERCHARGED : ecs::Projectile::BASIC;
        float damage = isSupercharged ? 50.0f : 25.0f;
        int pierce = isSupercharged ? 3 : 1;
        float hitboxRadius = isSupercharged ? 8.0f : 5.0f; // Match GameConfig::Hitbox values

        ecs::Entity projectileEntity =
            m_registry.createEntity()
                .with<ecs::Transform>("projectile_transform_" + std::to_string(projectileId), x, y, 0.0f)
                .with<ecs::Velocity>("projectile_velocity_" + std::to_string(projectileId), vx, vy)
                .with<ecs::Projectile>("projectile_" + std::to_string(projectileId), projType, damage, 5.0f, 0.0f,
                                       pierce)
                .with<ecs::Hitbox>("projectile_hitbox_" + std::to_string(projectileId), hitboxRadius, 0.0f, 0.0f)
                .build();

        m_projectileEntities[projectileId] = projectileEntity;
        registerEntity(projectileEntity, ServerEntityType::PROJECTILE_PLAYER, projectileId, 5.0f, playerId);

        return projectileEntity;
    }

    ecs::Entity EntityManager::createEnemyProjectile(std::uint32_t enemyId, float x, float y, float vx, float vy)
    {
        std::uint32_t projectileId = m_nextProjectileId++;

        ecs::Entity projectileEntity =
            m_registry.createEntity()
                .with<ecs::Transform>("enemy_projectile_transform_" + std::to_string(projectileId), x, y, 0.0f)
                .with<ecs::Velocity>("enemy_projectile_velocity_" + std::to_string(projectileId), vx, vy)
                .with<ecs::Projectile>("enemy_projectile_" + std::to_string(projectileId), ecs::Projectile::BASIC,
                                       15.0f, 10.0f, 0.0f, 1)
                .with<ecs::Hitbox>("enemy_projectile_hitbox_" + std::to_string(projectileId), 5.0f, 0.0f, 0.0f)
                .build();

        m_projectileEntities[projectileId] = projectileEntity;
        registerEntity(projectileEntity, ServerEntityType::PROJECTILE_ENEMY, projectileId, 10.0f, enemyId);

        return projectileEntity;
    }

    void EntityManager::destroyProjectile(std::uint32_t projectileId)
    {
        auto *metadata = getEntityMetadata(projectileId);
        if (metadata)
        {
            metadata->isActive = false;
        }
        m_destroyQueue.push_back(projectileId);
    }

    ecs::Entity EntityManager::getProjectile(std::uint32_t projectileId)
    {
        auto it = m_projectileEntities.find(projectileId);
        return (it != m_projectileEntities.end()) ? it->second : ecs::INVALID_ENTITY;
    }

    void EntityManager::updateLifetimes(float deltaTime)
    {
        for (auto &[networkId, metadata] : m_entityMetadata)
        {
            if (metadata.lifetime > 0.0f && metadata.isActive)
            {
                metadata.currentLife += deltaTime;
                if (metadata.currentLife >= metadata.lifetime)
                {
                    // Mark for destruction
                    m_destroyQueue.push_back(networkId);
                }
            }
        }
    }

    void EntityManager::cleanupDestroyedEntities() { processDestroyQueue(); }

    EntityMetadata *EntityManager::getEntityMetadata(std::uint32_t networkId)
    {
        auto it = m_entityMetadata.find(networkId);
        return (it != m_entityMetadata.end()) ? &it->second : nullptr;
    }

    const EntityMetadata *EntityManager::getEntityMetadata(std::uint32_t networkId) const
    {
        auto it = m_entityMetadata.find(networkId);
        return (it != m_entityMetadata.end()) ? &it->second : nullptr;
    }

    std::uint32_t EntityManager::getNetworkIdForEntity(ecs::Entity entity) const
    {
        auto it = m_entityToNetworkId.find(entity);
        return (it != m_entityToNetworkId.end()) ? it->second : 0;
    }

    std::vector<rnp::EntityState> EntityManager::getAllEntityStates() const
    {
        std::vector<rnp::EntityState> states;

        auto addEntityState = [&](std::uint32_t networkId, ecs::Entity entity, rnp::EntityType netType,
                                  rnp::EntitySubtype subtype = rnp::EntitySubtype::NONE)
        {
            auto *transform = m_registry.getComponent<ecs::Transform>(entity);
            auto *velocity = m_registry.getComponent<ecs::Velocity>(entity);

            if (transform && velocity)
            {
                rnp::EntityState state;
                state.id = networkId;
                state.type = static_cast<std::uint16_t>(netType);
                state.subtype = static_cast<std::uint8_t>(subtype);
                state.x = transform->x;
                state.y = transform->y;
                state.vx = velocity->x;
                state.vy = velocity->y;

                // Handle health percentage
                state.healthPercent = 255; // Default: no health bar

                if (netType == rnp::EntityType::PLAYER)
                {
                    auto *health = m_registry.getComponent<ecs::Health>(entity);
                    if (health && health->max > 0.0f)
                    {
                        float healthPct = (health->current / health->max) * 100.0f;
                        state.healthPercent = static_cast<std::uint8_t>(std::max(0.0f, std::min(100.0f, healthPct)));
                    }
                }
                else if (netType == rnp::EntityType::ENEMY || netType == rnp::EntityType::BOSS)
                {
                    auto *enemy = m_registry.getComponent<ecs::Enemy>(entity);
                    if (enemy && enemy->max_health > 0.0f)
                    {
                        float healthPct = (enemy->health / enemy->max_health) * 100.0f;
                        state.healthPercent = static_cast<std::uint8_t>(std::max(0.0f, std::min(100.0f, healthPct)));
                    }
                }

                // Handle special state flags
                state.stateFlags = 0;
                if (netType == rnp::EntityType::PLAYER)
                {
                    auto *beamCharge = m_registry.getComponent<ecs::BeamCharge>(entity);
                    if (beamCharge)
                    {
                        state.stateFlags = static_cast<std::uint8_t>(beamCharge->current_charge * 255.0f);
                    }
                }

                // Initialize score to 0 (will be set for players below)
                state.score = 0;

                states.push_back(state);
            }
        };

        for (const auto &[sessionId, entity] : m_playerEntities)
        {
            const auto *metadata = getEntityMetadata(sessionId);
            if (metadata && metadata->isActive)
            {
                addEntityState(sessionId, entity, rnp::EntityType::PLAYER);
                // Set the player's score in the last added state
                if (!states.empty())
                {
                    states.back().score = static_cast<std::uint32_t>(getScore(sessionId));
                }
            }
        }

        for (const auto &[enemyId, entity] : m_enemyEntities)
        {
            const auto *metadata = getEntityMetadata(enemyId);
            if (metadata && metadata->isActive)
            {
                rnp::EntityType netType;
                rnp::EntitySubtype subtype;

                if (metadata->type == ServerEntityType::BOSS)
                {
                    netType = rnp::EntityType::BOSS;
                    subtype = rnp::EntitySubtype::ENEMY_BOSS;
                }
                else if (metadata->type == ServerEntityType::ENEMY_ADVANCED)
                {
                    netType = rnp::EntityType::ENEMY;
                    subtype = rnp::EntitySubtype::ENEMY_ADVANCED;
                }
                else // ENEMY_BASIC
                {
                    netType = rnp::EntityType::ENEMY;
                    subtype = rnp::EntitySubtype::ENEMY_BASIC;
                }

                addEntityState(enemyId, entity, netType, subtype);
            }
        }

        for (const auto &[projectileId, entity] : m_projectileEntities)
        {
            const auto *metadata = getEntityMetadata(projectileId);
            if (metadata && metadata->isActive)
            {
                rnp::EntitySubtype subtype = rnp::EntitySubtype::NONE;

                if (metadata->type == ServerEntityType::PROJECTILE_PLAYER)
                {
                    // Check if supercharged
                    auto *projectile = m_registry.getComponent<ecs::Projectile>(entity);
                    if (projectile && projectile->type == ecs::Projectile::SUPERCHARGED)
                    {
                        subtype = rnp::EntitySubtype::PROJECTILE_PLAYER_SUPERCHARGED;
                    }
                    else
                    {
                        subtype = rnp::EntitySubtype::PROJECTILE_PLAYER;
                    }
                }
                else if (metadata->type == ServerEntityType::PROJECTILE_ENEMY)
                {
                    subtype = rnp::EntitySubtype::PROJECTILE_ENEMY;
                }

                addEntityState(projectileId, entity, rnp::EntityType::PROJECTILE, subtype);
            }
        }

        return states;
    }

    rnp::EntityType EntityManager::getNetworkEntityType(ServerEntityType type) const
    {
        switch (type)
        {
            case ServerEntityType::PLAYER:
                return rnp::EntityType::PLAYER;
            case ServerEntityType::ENEMY_BASIC:
            case ServerEntityType::ENEMY_ADVANCED:
                return rnp::EntityType::ENEMY;
            case ServerEntityType::BOSS:
                return rnp::EntityType::BOSS;
            case ServerEntityType::PROJECTILE_PLAYER:
            case ServerEntityType::PROJECTILE_ENEMY:
                return rnp::EntityType::PROJECTILE;
            default:
                return rnp::EntityType::PLAYER; // Default fallback
        }
    }

    void EntityManager::clear()
    {
        // Clear all players
        for (auto &[sessionId, entity] : m_playerEntities)
        {
            if (m_registry.hasComponent<ecs::Transform>(entity))
                m_registry.removeComponent<ecs::Transform>(entity);
            if (m_registry.hasComponent<ecs::Velocity>(entity))
                m_registry.removeComponent<ecs::Velocity>(entity);
            if (m_registry.hasComponent<ecs::Player>(entity))
                m_registry.removeComponent<ecs::Player>(entity);
            if (m_registry.hasComponent<ecs::BeamCharge>(entity))
                m_registry.removeComponent<ecs::BeamCharge>(entity);
            if (m_registry.hasComponent<ecs::Hitbox>(entity))
                m_registry.removeComponent<ecs::Hitbox>(entity);
            if (m_registry.hasComponent<ecs::Health>(entity))
                m_registry.removeComponent<ecs::Health>(entity);
        }
        m_playerEntities.clear();

        // Clear all enemies
        for (auto &[enemyId, entity] : m_enemyEntities)
        {
            if (m_registry.hasComponent<ecs::Transform>(entity))
                m_registry.removeComponent<ecs::Transform>(entity);
            if (m_registry.hasComponent<ecs::Velocity>(entity))
                m_registry.removeComponent<ecs::Velocity>(entity);
            if (m_registry.hasComponent<ecs::Enemy>(entity))
                m_registry.removeComponent<ecs::Enemy>(entity);
            if (m_registry.hasComponent<ecs::Hitbox>(entity))
                m_registry.removeComponent<ecs::Hitbox>(entity);
        }
        m_enemyEntities.clear();

        // Clear all projectiles
        for (auto &[projectileId, entity] : m_projectileEntities)
        {
            if (m_registry.hasComponent<ecs::Transform>(entity))
                m_registry.removeComponent<ecs::Transform>(entity);
            if (m_registry.hasComponent<ecs::Velocity>(entity))
                m_registry.removeComponent<ecs::Velocity>(entity);
            if (m_registry.hasComponent<ecs::Projectile>(entity))
                m_registry.removeComponent<ecs::Projectile>(entity);
            if (m_registry.hasComponent<ecs::Hitbox>(entity))
                m_registry.removeComponent<ecs::Hitbox>(entity);
        }
        m_projectileEntities.clear();

        // Clear all powerups
        for (auto &[powerupId, entity] : m_powerupEntities)
        {
            if (m_registry.hasComponent<ecs::Transform>(entity))
                m_registry.removeComponent<ecs::Transform>(entity);
            if (m_registry.hasComponent<ecs::Velocity>(entity))
                m_registry.removeComponent<ecs::Velocity>(entity);
            if (m_registry.hasComponent<ecs::Hitbox>(entity))
                m_registry.removeComponent<ecs::Hitbox>(entity);
        }
        m_powerupEntities.clear();

        m_entityMetadata.clear();
        m_entityToNetworkId.clear();
        m_destroyQueue.clear();
        m_deadPlayers.clear();
    }

    size_t EntityManager::getTotalEntityCount() const
    {
        return m_playerEntities.size() + m_enemyEntities.size() + m_projectileEntities.size() +
               m_powerupEntities.size();
    }

    void EntityManager::registerEntity(ecs::Entity entity, ServerEntityType type, std::uint32_t networkId,
                                       float lifetime, std::uint32_t ownerId)
    {
        EntityMetadata metadata;
        metadata.entity = entity;
        metadata.type = type;
        metadata.networkId = networkId;
        metadata.isActive = true;
        metadata.lifetime = lifetime;
        metadata.currentLife = 0.0f;
        metadata.ownerId = ownerId;

        m_entityMetadata[networkId] = metadata;
        m_entityToNetworkId[entity] = networkId;
    }

    void EntityManager::unregisterEntity(std::uint32_t networkId)
    {
        auto it = m_entityMetadata.find(networkId);
        if (it != m_entityMetadata.end())
        {
            m_entityToNetworkId.erase(it->second.entity);
            m_entityMetadata.erase(it);
        }
    }

    void EntityManager::processDestroyQueue()
    {
        for (std::uint32_t networkId : m_destroyQueue)
        {
            // Check which container this entity belongs to
            auto playerIt = m_playerEntities.find(networkId);
            if (playerIt != m_playerEntities.end())
            {
                ecs::Entity entity = playerIt->second;

                // Remove components
                if (m_registry.hasComponent<ecs::Transform>(entity))
                    m_registry.removeComponent<ecs::Transform>(entity);
                if (m_registry.hasComponent<ecs::Velocity>(entity))
                    m_registry.removeComponent<ecs::Velocity>(entity);
                if (m_registry.hasComponent<ecs::Player>(entity))
                    m_registry.removeComponent<ecs::Player>(entity);
                if (m_registry.hasComponent<ecs::Health>(entity))
                    m_registry.removeComponent<ecs::Health>(entity);
                if (m_registry.hasComponent<ecs::BeamCharge>(entity))
                    m_registry.removeComponent<ecs::BeamCharge>(entity);
                if (m_registry.hasComponent<ecs::Hitbox>(entity))
                    m_registry.removeComponent<ecs::Hitbox>(entity);

                unregisterEntity(networkId);

                // Clear score and other player data
                m_playerScores.erase(networkId);
                m_deadPlayers.erase(networkId);
                m_playerEntities.erase(playerIt);

                utl::Logger::log("EntityManager: Destroyed player entity for sessionId " + std::to_string(networkId),
                                 utl::LogLevel::INFO);
                continue;
            }

            auto enemyIt = m_enemyEntities.find(networkId);
            if (enemyIt != m_enemyEntities.end())
            {
                ecs::Entity entity = enemyIt->second;
                if (m_registry.hasComponent<ecs::Transform>(entity))
                    m_registry.removeComponent<ecs::Transform>(entity);
                if (m_registry.hasComponent<ecs::Velocity>(entity))
                    m_registry.removeComponent<ecs::Velocity>(entity);
                if (m_registry.hasComponent<ecs::Enemy>(entity))
                    m_registry.removeComponent<ecs::Enemy>(entity);
                if (m_registry.hasComponent<ecs::Hitbox>(entity))
                    m_registry.removeComponent<ecs::Hitbox>(entity);

                unregisterEntity(networkId);
                m_enemyEntities.erase(enemyIt);
                continue;
            }

            auto projIt = m_projectileEntities.find(networkId);
            if (projIt != m_projectileEntities.end())
            {
                ecs::Entity entity = projIt->second;
                if (m_registry.hasComponent<ecs::Transform>(entity))
                    m_registry.removeComponent<ecs::Transform>(entity);
                if (m_registry.hasComponent<ecs::Velocity>(entity))
                    m_registry.removeComponent<ecs::Velocity>(entity);
                if (m_registry.hasComponent<ecs::Projectile>(entity))
                    m_registry.removeComponent<ecs::Projectile>(entity);
                if (m_registry.hasComponent<ecs::Hitbox>(entity))
                    m_registry.removeComponent<ecs::Hitbox>(entity);

                unregisterEntity(networkId);
                m_projectileEntities.erase(projIt);
                continue;
            }

            auto powerupIt = m_powerupEntities.find(networkId);
            if (powerupIt != m_powerupEntities.end())
            {
                ecs::Entity entity = powerupIt->second;
                if (m_registry.hasComponent<ecs::Transform>(entity))
                    m_registry.removeComponent<ecs::Transform>(entity);
                if (m_registry.hasComponent<ecs::Velocity>(entity))
                    m_registry.removeComponent<ecs::Velocity>(entity);
                if (m_registry.hasComponent<ecs::Hitbox>(entity))
                    m_registry.removeComponent<ecs::Hitbox>(entity);

                unregisterEntity(networkId);
                m_powerupEntities.erase(powerupIt);
                continue;
            }
        }

        m_destroyQueue.clear();
    }

} // namespace gme
