///
/// @file CollisionSystem.hpp
/// @brief Server-side collision detection system
/// @namespace gme
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
    /// @brief Collision pair for tracking collisions
    ///
    struct CollisionPair
    {
            std::uint32_t entityId1;
            std::uint32_t entityId2;
            float overlapDistance;
    };

    ///
    /// @class CollisionSystem
    /// @brief Handles collision detection and resolution on server
    /// @namespace gme
    ///
    class CollisionSystem final : public ecs::ASystem
    {
        public:
            explicit CollisionSystem(ecs::Registry &registry, EntityManager &entityManager)
                : m_registry(registry), m_entityManager(entityManager)
            {
            }
            ~CollisionSystem() override = default;

            CollisionSystem(const CollisionSystem &) = delete;
            CollisionSystem &operator=(const CollisionSystem &) = delete;
            CollisionSystem(CollisionSystem &&) = delete;
            CollisionSystem &operator=(CollisionSystem &&) = delete;

            void update(ecs::Registry &registry, float deltaTime) override;

            // Collision statistics
            size_t getCollisionCount() const { return m_collisionCount; }
            void resetCollisionCount() { m_collisionCount = 0; }

        private:
            ecs::Registry &m_registry;
            EntityManager &m_entityManager;
            size_t m_collisionCount = 0;

            // Collision detection helpers
            bool checkCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2,
                                      float *overlapDist = nullptr) const;

            // Collision handlers for different entity type combinations
            void handlePlayerProjectileEnemyCollision();
            void handleEnemyProjectilePlayerCollision();
            void handlePlayerEnemyCollision();
            void handlePlayerPowerupCollision();

            // Helper to get collision info
            bool getCollisionInfo(ecs::Entity entity, float &x, float &y, float &radius) const;

            // Damage application
            void applyDamageToEnemy(std::uint32_t enemyId, float damage);
            void applyDamageToPlayer(std::uint32_t playerId, float damage);
    };

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
