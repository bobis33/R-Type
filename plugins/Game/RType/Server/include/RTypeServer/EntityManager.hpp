///
/// @file EntityManager.hpp
/// @brief Centralized entity lifecycle management system for R-Type server
/// @details This file contains the EntityManager class which provides a complete entity
///          management system for the R-Type game server. It handles creation, destruction,
///          tracking, and synchronization of all game entities including players, enemies,
///          projectiles, and powerups. The manager maintains network IDs for entity
///          synchronization and provides factory methods for entity creation with proper
///          component initialization.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
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
    /// @enum ServerEntityType
    /// @brief Enumeration of entity types managed by the server
    /// @details Categorizes entities for proper handling, network synchronization,
    ///          and behavior assignment. Each type has specific properties and behaviors.
    ///
    enum class ServerEntityType : uint8_t
    {
        PLAYER = 0,            ///< Player-controlled entity
        ENEMY_BASIC = 1,       ///< Basic enemy type (simple behavior)
        ENEMY_ADVANCED = 2,    ///< Advanced enemy type (complex behavior)
        BOSS = 3,              ///< Boss enemy (high health, special patterns)
        PROJECTILE_PLAYER = 4, ///< Player-fired projectile
        PROJECTILE_ENEMY = 5,  ///< Enemy-fired projectile
        POWERUP = 6            ///< Collectible powerup item
    };

    ///
    /// @struct EntityMetadata
    /// @brief Metadata structure for tracking entity lifecycle and properties
    /// @details Stores additional information about entities beyond ECS components,
    ///          including network synchronization data, lifetime management, and ownership.
    ///
    struct EntityMetadata
    {
            ecs::Entity entity;      ///< ECS entity handle
            ServerEntityType type;   ///< Entity type classification
            std::uint32_t networkId; ///< Unique ID used in network synchronization packets
            bool isActive;           ///< Whether entity is currently active
            float lifetime;          ///< Maximum lifetime in seconds (-1 for infinite)
            float currentLife;       ///< Elapsed lifetime counter in seconds
            std::uint32_t ownerId;   ///< Owner entity ID (for projectiles, the spawner)
    };

    ///
    /// @class EntityManager
    /// @brief Central entity lifecycle manager for the R-Type game server
    /// @details This class provides comprehensive entity management including:
    ///          - Entity creation with proper component initialization
    ///          - Entity destruction and cleanup
    ///          - Network ID assignment and tracking
    ///          - Lifetime management for temporary entities
    ///          - Player score tracking
    ///          - Network state synchronization
    ///          - Reverse lookups (entity <-> network ID)
    ///
    /// The manager maintains separate containers for different entity types and ensures
    /// proper cleanup of destroyed entities. All entities are assigned unique network IDs
    /// for client-server synchronization.
    ///
    /// @namespace gme
    ///
    class EntityManager
    {
        public:
            ///
            /// @brief Constructor
            /// @param registry Reference to the ECS registry
            /// @details Initializes the entity manager with a reference to the game's ECS registry
            ///
            explicit EntityManager(ecs::Registry &registry);

            ///
            /// @brief Destructor
            ///
            ~EntityManager() = default;

            /// @brief Deleted copy constructor (non-copyable)
            EntityManager(const EntityManager &) = delete;
            /// @brief Deleted copy assignment operator (non-copyable)
            EntityManager &operator=(const EntityManager &) = delete;
            /// @brief Deleted move constructor (non-movable)
            EntityManager(EntityManager &&) = delete;
            /// @brief Deleted move assignment operator (non-movable)
            EntityManager &operator=(EntityManager &&) = delete;

            // ========== Player Management ==========

            ///
            /// @brief Create a new player entity
            /// @param sessionId Network session ID of the player
            /// @param x Initial X position
            /// @param y Initial Y position
            /// @return Created player entity handle
            /// @details Creates a player entity with all required components (Transform, Velocity,
            ///          Health, Hitbox, NetworkId) and registers it for tracking
            ///
            ecs::Entity createPlayer(std::uint32_t sessionId, float x, float y);

            ///
            /// @brief Destroy a player entity
            /// @param sessionId Network session ID of the player to destroy
            /// @details Removes player from tracking and queues entity for destruction
            ///
            void destroyPlayer(std::uint32_t sessionId);

            ///
            /// @brief Get player entity by session ID
            /// @param sessionId Network session ID
            /// @return Player entity handle (may be invalid if player doesn't exist)
            ///
            ecs::Entity getPlayer(std::uint32_t sessionId);

            ///
            /// @brief Get player entity by session ID (const version)
            /// @param sessionId Network session ID
            /// @return Player entity handle
            ///
            ecs::Entity getPlayerEntity(std::uint32_t sessionId) const;

            ///
            /// @brief Check if player exists
            /// @param sessionId Network session ID
            /// @return True if player exists and is registered
            ///
            bool hasPlayer(std::uint32_t sessionId) const;

            ///
            /// @brief Mark a player as dead without destroying the entity
            /// @param sessionId Network session ID
            /// @details Adds player to dead players set, useful for game over detection
            ///
            void markPlayerAsDead(std::uint32_t sessionId);

            ///
            /// @brief Get count of alive players
            /// @return Number of players not marked as dead
            ///
            std::uint32_t getAlivePlayerCount() const;

            ///
            /// @brief Get all player entities
            /// @return Const reference to map of session ID to player entity
            ///
            const std::unordered_map<std::uint32_t, ecs::Entity> &getPlayers() const { return m_playerEntities; }

            ///
            /// @brief Add points to a player's score
            /// @param sessionId Network session ID of the player
            /// @param points Points to add (can be negative)
            ///
            void addScore(std::uint32_t sessionId, int points);

            ///
            /// @brief Get player's current score
            /// @param sessionId Network session ID
            /// @return Current score (0 if player not found)
            ///
            int getScore(std::uint32_t sessionId) const;

            ///
            /// @brief Reset player's score to zero
            /// @param sessionId Network session ID
            ///
            void resetScore(std::uint32_t sessionId);

            ///
            /// @brief Create a basic enemy entity
            /// @param x Spawn X position
            /// @param y Spawn Y position
            /// @param health Initial health (default: 50.0)
            /// @return Created enemy entity handle
            /// @details Creates basic enemy with simple AI behavior
            ///
            ecs::Entity createBasicEnemy(float x, float y, float health = 50.0f);

            ///
            /// @brief Create an advanced enemy entity
            /// @param x Spawn X position
            /// @param y Spawn Y position
            /// @param health Initial health (default: 100.0)
            /// @return Created enemy entity handle
            /// @details Creates advanced enemy with complex AI behavior
            ///
            ecs::Entity createAdvancedEnemy(float x, float y, float health = 100.0f);

            ///
            /// @brief Create a boss enemy entity
            /// @param x Spawn X position
            /// @param y Spawn Y position
            /// @param health Initial health (default: 1000.0)
            /// @return Created boss entity handle
            /// @details Creates boss with special AI and high durability
            ///
            ecs::Entity createBoss(float x, float y, float health = 1000.0f);

            ///
            /// @brief Destroy an enemy entity
            /// @param enemyId Network ID of the enemy
            ///
            void destroyEnemy(std::uint32_t enemyId);

            ///
            /// @brief Get enemy entity by network ID
            /// @param enemyId Network ID of the enemy
            /// @return Enemy entity handle
            ///
            ecs::Entity getEnemy(std::uint32_t enemyId);

            ///
            /// @brief Get all enemy entities
            /// @return Const reference to map of network ID to enemy entity
            ///
            const std::unordered_map<std::uint32_t, ecs::Entity> &getEnemies() const { return m_enemyEntities; }

            ///
            /// @brief Create a player projectile entity
            /// @param playerId Session ID of the player who fired
            /// @param x Initial X position
            /// @param y Initial Y position
            /// @param vx X velocity
            /// @param vy Y velocity
            /// @param isSupercharged Whether this is a powered-up shot (default: false)
            /// @return Created projectile entity handle
            ///
            ecs::Entity createPlayerProjectile(std::uint32_t playerId, float x, float y, float vx, float vy,
                                               bool isSupercharged = false);

            ///
            /// @brief Create an enemy projectile entity
            /// @param enemyId Network ID of the enemy who fired
            /// @param x Initial X position
            /// @param y Initial Y position
            /// @param vx X velocity
            /// @param vy Y velocity
            /// @return Created projectile entity handle
            ///
            ecs::Entity createEnemyProjectile(std::uint32_t enemyId, float x, float y, float vx, float vy);

            ///
            /// @brief Destroy a projectile entity
            /// @param projectileId Network ID of the projectile
            ///
            void destroyProjectile(std::uint32_t projectileId);

            ///
            /// @brief Get projectile entity by network ID
            /// @param projectileId Network ID of the projectile
            /// @return Projectile entity handle
            ///
            ecs::Entity getProjectile(std::uint32_t projectileId);

            ///
            /// @brief Get all projectile entities
            /// @return Const reference to map of network ID to projectile entity
            ///
            const std::unordered_map<std::uint32_t, ecs::Entity> &getProjectiles() const
            {
                return m_projectileEntities;
            }

            ///
            /// @brief Create a powerup entity
            /// @param x Spawn X position
            /// @param y Spawn Y position
            /// @param powerupType Type of powerup (0-255)
            /// @return Created powerup entity handle
            ///
            ecs::Entity createPowerup(float x, float y, uint8_t powerupType);

            ///
            /// @brief Destroy a powerup entity
            /// @param powerupId Network ID of the powerup
            ///
            void destroyPowerup(std::uint32_t powerupId);

            ///
            /// @brief Update lifetime counters for temporary entities
            /// @param deltaTime Time elapsed since last frame (seconds)
            /// @details Increments currentLife for entities with limited lifetime and destroys
            ///          entities that have exceeded their lifetime
            ///
            void updateLifetimes(float deltaTime);

            ///
            /// @brief Process destruction queue and remove destroyed entities
            /// @details Should be called after all systems have updated to safely remove entities
            ///
            void cleanupDestroyedEntities();

            ///
            /// @brief Get entity metadata by network ID
            /// @param networkId Network ID of the entity
            /// @return Pointer to metadata (nullptr if not found)
            ///
            EntityMetadata *getEntityMetadata(std::uint32_t networkId);

            ///
            /// @brief Get entity metadata by network ID (const version)
            /// @param networkId Network ID of the entity
            /// @return Const pointer to metadata (nullptr if not found)
            ///
            const EntityMetadata *getEntityMetadata(std::uint32_t networkId) const;

            ///
            /// @brief Get network ID for an entity handle
            /// @param entity ECS entity handle
            /// @return Network ID (0 if not found)
            ///
            std::uint32_t getNetworkIdForEntity(ecs::Entity entity) const;

            ///
            /// @brief Get network state for all entities
            /// @return Vector of EntityState structures for network transmission
            /// @details Creates snapshot of all entity positions, velocities, and states
            ///          for synchronization with clients
            ///
            std::vector<rnp::EntityState> getAllEntityStates() const;

            ///
            /// @brief Convert server entity type to network protocol entity type
            /// @param type Server entity type
            /// @return Corresponding network protocol entity type
            ///
            rnp::EntityType getNetworkEntityType(ServerEntityType type) const;

            ///
            /// @brief Clear all entities and reset manager state
            /// @details Destroys all entities and resets ID generators
            ///
            void clear();

            ///
            /// @brief Get total count of all managed entities
            /// @return Total number of active entities
            ///
            size_t getTotalEntityCount() const;

        private:
            ecs::Registry &m_registry; ///< Reference to ECS registry

            // Entity containers (networkId -> Entity)
            std::unordered_map<std::uint32_t, ecs::Entity> m_playerEntities;     ///< Player entities by session ID
            std::unordered_map<std::uint32_t, ecs::Entity> m_enemyEntities;      ///< Enemy entities by network ID
            std::unordered_map<std::uint32_t, ecs::Entity> m_projectileEntities; ///< Projectile entities by network ID
            std::unordered_map<std::uint32_t, ecs::Entity> m_powerupEntities;    ///< Powerup entities by network ID

            // Metadata tracking (networkId -> Metadata)
            std::unordered_map<std::uint32_t, EntityMetadata> m_entityMetadata; ///< Entity metadata by network ID

            // Dead players tracking
            std::unordered_set<std::uint32_t> m_deadPlayers; ///< Set of dead player session IDs

            // Score tracking (sessionId -> score)
            std::unordered_map<std::uint32_t, int> m_playerScores; ///< Player scores by session ID

            // Reverse lookup (Entity -> networkId)
            std::unordered_map<ecs::Entity, std::uint32_t> m_entityToNetworkId; ///< Reverse lookup map

            // ID generators
            std::uint32_t m_nextEnemyId = 2000;      ///< Next available enemy network ID
            std::uint32_t m_nextProjectileId = 1000; ///< Next available projectile network ID
            std::uint32_t m_nextPowerupId = 5000;    ///< Next available powerup network ID

            // Destruction queue (to avoid modifying containers during iteration)
            std::vector<std::uint32_t> m_destroyQueue; ///< Queue of network IDs pending destruction

            ///
            /// @brief Register a new entity with the manager
            /// @param entity ECS entity handle
            /// @param type Entity type classification
            /// @param networkId Network ID for synchronization
            /// @param lifetime Maximum lifetime in seconds (-1.0 for infinite)
            /// @param ownerId Owner entity ID (for projectiles)
            ///
            void registerEntity(ecs::Entity entity, ServerEntityType type, std::uint32_t networkId,
                                float lifetime = -1.0f, std::uint32_t ownerId = 0);

            ///
            /// @brief Unregister an entity from the manager
            /// @param networkId Network ID of the entity to unregister
            ///
            void unregisterEntity(std::uint32_t networkId);

            ///
            /// @brief Process pending entity destructions from queue
            /// @details Safely removes entities that were queued for destruction
            ///
            void processDestroyQueue();
    };

} // namespace gme
