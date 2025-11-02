/// @file EntityManager.hpp
/// @brief Centralized entity management for RTypeServer
/// @namespace gme
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gme
{
    ///
    /// @brief Entity type enumeration for server-side management
    ///
    enum class ServerEntityType : uint8_t
    {
        PLAYER = 0,
        ENEMY_BASIC = 1,
        ENEMY_ADVANCED = 2,
        BOSS = 3,
        PROJECTILE_PLAYER = 4,
        PROJECTILE_ENEMY = 5,
        POWERUP = 6
    };

    ///
    /// @brief Entity metadata for tracking
    ///
    struct EntityMetadata
    {
            ecs::Entity entity;
            ServerEntityType type;
            std::uint32_t networkId; // ID used in network packets
            bool isActive;
            float lifetime;        // -1 for infinite
            float currentLife;     // Current lifetime counter
            std::uint32_t ownerId; // For projectiles, the entity that spawned it
    };

    ///
    /// @class EntityManager
    /// @brief Manages entity lifecycle and provides factory methods
    /// @namespace gme
    ///
    class EntityManager
    {
        public:
            explicit EntityManager(ecs::Registry &registry);
            ~EntityManager() = default;

            EntityManager(const EntityManager &) = delete;
            EntityManager &operator=(const EntityManager &) = delete;
            EntityManager(EntityManager &&) = delete;
            EntityManager &operator=(EntityManager &&) = delete;

            // ========== Player Management ==========
            ecs::Entity createPlayer(std::uint32_t sessionId, float x, float y);
            void destroyPlayer(std::uint32_t sessionId);
            ecs::Entity getPlayer(std::uint32_t sessionId);
            ecs::Entity getPlayerEntity(std::uint32_t sessionId) const;
            bool hasPlayer(std::uint32_t sessionId) const;
            void markPlayerAsDead(std::uint32_t sessionId);
            std::uint32_t getAlivePlayerCount() const;
            const std::unordered_map<std::uint32_t, ecs::Entity> &getPlayers() const { return m_playerEntities; }

            // ========== Score Management ==========
            void addScore(std::uint32_t sessionId, int points);
            int getScore(std::uint32_t sessionId) const;
            void resetScore(std::uint32_t sessionId);

            // ========== Enemy Management ==========
            ecs::Entity createBasicEnemy(float x, float y, float health = 50.0f);
            ecs::Entity createAdvancedEnemy(float x, float y, float health = 100.0f);
            ecs::Entity createBoss(float x, float y, float health = 1000.0f);
            void destroyEnemy(std::uint32_t enemyId);
            ecs::Entity getEnemy(std::uint32_t enemyId);
            const std::unordered_map<std::uint32_t, ecs::Entity> &getEnemies() const { return m_enemyEntities; }

            // ========== Projectile Management ==========
            ecs::Entity createPlayerProjectile(std::uint32_t playerId, float x, float y, float vx, float vy,
                                               bool isSupercharged = false);
            ecs::Entity createEnemyProjectile(std::uint32_t enemyId, float x, float y, float vx, float vy);
            void destroyProjectile(std::uint32_t projectileId);
            ecs::Entity getProjectile(std::uint32_t projectileId);
            const std::unordered_map<std::uint32_t, ecs::Entity> &getProjectiles() const
            {
                return m_projectileEntities;
            }

            // ========== Powerup Management ==========
            ecs::Entity createPowerup(float x, float y, uint8_t powerupType);
            void destroyPowerup(std::uint32_t powerupId);

            // ========== Generic Entity Management ==========
            void updateLifetimes(float deltaTime);
            void cleanupDestroyedEntities();
            EntityMetadata *getEntityMetadata(std::uint32_t networkId);
            const EntityMetadata *getEntityMetadata(std::uint32_t networkId) const;
            std::uint32_t getNetworkIdForEntity(ecs::Entity entity) const;

            // ========== Network Sync ==========
            std::vector<rnp::EntityState> getAllEntityStates() const;
            rnp::EntityType getNetworkEntityType(ServerEntityType type) const;

            // ========== Utility ==========
            void clear();
            size_t getTotalEntityCount() const;

        private:
            ecs::Registry &m_registry;

            // Entity containers (networkId -> Entity)
            std::unordered_map<std::uint32_t, ecs::Entity> m_playerEntities;
            std::unordered_map<std::uint32_t, ecs::Entity> m_enemyEntities;
            std::unordered_map<std::uint32_t, ecs::Entity> m_projectileEntities;
            std::unordered_map<std::uint32_t, ecs::Entity> m_powerupEntities;

            // Metadata tracking (networkId -> Metadata)
            std::unordered_map<std::uint32_t, EntityMetadata> m_entityMetadata;

            // Dead players tracking
            std::unordered_set<std::uint32_t> m_deadPlayers;

            // Score tracking (sessionId -> score)
            std::unordered_map<std::uint32_t, int> m_playerScores;

            // Reverse lookup (Entity -> networkId)
            std::unordered_map<ecs::Entity, std::uint32_t> m_entityToNetworkId;

            // ID generators
            std::uint32_t m_nextEnemyId = 2000;
            std::uint32_t m_nextProjectileId = 1000;
            std::uint32_t m_nextPowerupId = 5000;

            // Destruction queue (to avoid modifying containers during iteration)
            std::vector<std::uint32_t> m_destroyQueue;

            // Helper methods
            void registerEntity(ecs::Entity entity, ServerEntityType type, std::uint32_t networkId,
                                float lifetime = -1.0f, std::uint32_t ownerId = 0);
            void unregisterEntity(std::uint32_t networkId);
            void processDestroyQueue();
    };

} // namespace gme
