///
/// @file CollisionSystem.hpp
/// @brief Server-side collision detection and resolution system for R-Type
/// @details This file contains the collision detection system that runs on the game server.
///          It handles all collision detection between different entity types (players, enemies,
///          projectiles, powerups) and applies appropriate responses (damage, destruction, etc.).
///          The system uses circular collision detection for efficiency and accuracy.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "RTypeServer/EntityManager.hpp"
#include <cmath>
#include <vector>

namespace gme
{
    ///
    /// @struct CollisionPair
    /// @brief Data structure for storing collision information between two entities
    /// @details Used to track collisions that have been detected, including the entities
    ///          involved and the degree of overlap for potential resolution.
    ///
    struct CollisionPair
    {
            std::uint32_t entityId1; ///< First entity ID in collision
            std::uint32_t entityId2; ///< Second entity ID in collision
            float overlapDistance;   ///< Distance of overlap (radiusSum - distance)
    };

    ///
    /// @class CollisionSystem
    /// @brief Server-authoritative collision detection and response system
    /// @details This ECS system manages all collision detection and resolution on the server.
    ///          It performs the following operations each frame:
    ///          - Detects collisions between player projectiles and enemies
    ///          - Detects collisions between enemy projectiles and players
    ///          - Detects collisions between players and enemies (ramming)
    ///          - Detects collisions between players and powerups
    ///          - Applies damage to entities based on collision type
    ///          - Destroys entities when appropriate
    ///          - Tracks collision statistics for debugging
    ///
    /// The system uses circular collision detection (radius-based) which is computationally
    /// efficient for the types of entities in R-Type. All collision resolution is handled
    /// server-side to maintain authoritative game state.
    ///
    /// @namespace gme
    ///
    class CollisionSystem final : public ecs::ASystem
    {
        public:
            ///
            /// @brief Constructor
            /// @param registry ECS registry containing all entities and components
            /// @param entityManager Entity manager for spawning/destroying entities
            /// @details Initializes the collision system with references to required managers
            ///
            explicit CollisionSystem(ecs::Registry &registry, EntityManager &entityManager)
                : m_registry(registry), m_entityManager(entityManager)
            {
            }

            ///
            /// @brief Destructor
            ///
            ~CollisionSystem() override = default;

            /// @brief Deleted copy constructor (non-copyable)
            CollisionSystem(const CollisionSystem &) = delete;
            /// @brief Deleted copy assignment operator (non-copyable)
            CollisionSystem &operator=(const CollisionSystem &) = delete;
            /// @brief Deleted move constructor (non-movable)
            CollisionSystem(CollisionSystem &&) = delete;
            /// @brief Deleted move assignment operator (non-movable)
            CollisionSystem &operator=(CollisionSystem &&) = delete;

            ///
            /// @brief Update the collision system (called each frame)
            /// @param registry ECS registry containing all entities
            /// @param deltaTime Time elapsed since last frame (unused)
            /// @details Checks for collisions between all relevant entity pairs and applies
            ///          appropriate responses (damage, destruction, etc.)
            ///
            void update(ecs::Registry &registry, float deltaTime) override;

            ///
            /// @brief Get the number of collisions detected this frame
            /// @return Total collision count
            /// @details Useful for debugging and performance monitoring
            ///
            size_t getCollisionCount() const { return m_collisionCount; }

            ///
            /// @brief Reset the collision counter to zero
            /// @details Should be called periodically to prevent overflow
            ///
            void resetCollisionCount() { m_collisionCount = 0; }

        private:
            ecs::Registry &m_registry;      ///< ECS registry reference
            EntityManager &m_entityManager; ///< Entity manager reference for spawning/destruction
            size_t m_collisionCount = 0;    ///< Counter for collisions detected this frame

            ///
            /// @brief Check for circular collision between two entities
            /// @param x1 X position of first entity center
            /// @param y1 Y position of first entity center
            /// @param r1 Radius of first entity
            /// @param x2 X position of second entity center
            /// @param y2 Y position of second entity center
            /// @param r2 Radius of second entity
            /// @param overlapDist Optional output parameter for overlap distance
            /// @return True if entities are colliding
            /// @details Uses distance formula: sqrt((x2-x1)² + (y2-y1)²) < (r1 + r2)
            ///
            bool checkCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2,
                                      float *overlapDist = nullptr) const;

            ///
            /// @brief Handle collisions between player projectiles and enemies
            /// @details Iterates through all player projectiles and enemies, checks for collisions,
            ///          applies damage to enemies, and destroys projectiles on hit
            ///
            void handlePlayerProjectileEnemyCollision();

            ///
            /// @brief Handle collisions between enemy projectiles and players
            /// @details Iterates through all enemy projectiles and players, checks for collisions,
            ///          applies damage to players, and destroys projectiles on hit
            ///
            void handleEnemyProjectilePlayerCollision();

            ///
            /// @brief Handle collisions between players and enemies (ramming)
            /// @details Checks for direct collisions between players and enemies,
            ///          applies damage to both entities on contact
            ///
            void handlePlayerEnemyCollision();

            ///
            /// @brief Handle collisions between players and powerups
            /// @details Checks for collisions between players and powerup items,
            ///          applies powerup effects and destroys the powerup entity
            ///
            void handlePlayerPowerupCollision();

            ///
            /// @brief Extract collision information from an entity
            /// @param entity Entity to get collision info from
            /// @param x Output: X position of collision center
            /// @param y Output: Y position of collision center
            /// @param radius Output: Collision radius
            /// @return True if entity has valid Transform and Hitbox components
            /// @details Calculates collision center from transform position and hitbox offset
            ///
            bool getCollisionInfo(ecs::Entity entity, float &x, float &y, float &radius) const;

            ///
            /// @brief Apply damage to an enemy entity
            /// @param enemyId Network ID of the enemy entity
            /// @param damage Amount of damage to apply
            /// @param attackerPlayerId Session ID of player who dealt damage (for score attribution)
            /// @details Reduces enemy health and destroys entity if health reaches zero.
            ///          Awards score to attacker if enemy is destroyed.
            ///
            void applyDamageToEnemy(std::uint32_t enemyId, float damage, std::uint32_t attackerPlayerId = 0);

            ///
            /// @brief Apply damage to a player entity
            /// @param playerId Session ID of the player
            /// @param damage Amount of damage to apply
            /// @details Reduces player health. Player death is handled separately.
            ///
            void applyDamageToPlayer(std::uint32_t playerId, float damage);
    };

    ///
    /// @brief Inline implementation of circular collision detection
    /// @param x1 X position of first circle center
    /// @param y1 Y position of first circle center
    /// @param r1 Radius of first circle
    /// @param x2 X position of second circle center
    /// @param y2 Y position of second circle center
    /// @param r2 Radius of second circle
    /// @param overlapDist Optional pointer to store overlap distance (radiusSum - actualDistance)
    /// @return True if circles are overlapping (distance < radiusSum)
    /// @details Calculates Euclidean distance between centers and compares to sum of radii.
    ///          More efficient than AABB for circular entities.
    ///
    inline bool CollisionSystem::checkCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2,
                                                      float *overlapDist) const
    {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float distance = std::sqrt(dx * dx + dy * dy);
        float radiusSum = r1 + r2;

        if (overlapDist)
        {
            *overlapDist = radiusSum - distance;
        }

        return distance < radiusSum;
    }

    ///
    /// @brief Inline implementation of collision info extraction
    /// @param entity Entity to extract collision info from
    /// @param x Output parameter for collision center X position
    /// @param y Output parameter for collision center Y position
    /// @param radius Output parameter for collision radius
    /// @return True if entity has both Transform and Hitbox components, false otherwise
    /// @details Combines transform position with hitbox offset to calculate actual collision center
    ///
    inline bool CollisionSystem::getCollisionInfo(ecs::Entity entity, float &x, float &y, float &radius) const
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(entity);
        auto *hitbox = m_registry.getComponent<ecs::Hitbox>(entity);

        if (!transform || !hitbox)
            return false;

        x = transform->x + hitbox->offsetX;
        y = transform->y + hitbox->offsetY;
        radius = hitbox->radius;
        return true;
    }

} // namespace gme
