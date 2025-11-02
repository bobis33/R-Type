///
/// @file CollisionSystem.cpp
/// @brief Implementation of server-side collision detection system
/// @namespace gme
///

#include "RTypeServer/Systems/CollisionSystem.hpp"
#include "Utils/Logger.hpp"
#include <algorithm>

namespace gme
{
    void CollisionSystem::update(ecs::Registry &registry, float deltaTime)
    {
        (void)registry;  // Use member registry instead
        (void)deltaTime; // Collision detection is instantaneous

        m_collisionCount = 0;

        // Check all collision types
        handlePlayerProjectileEnemyCollision();
        handleEnemyProjectilePlayerCollision();
        handlePlayerEnemyCollision();
        handlePlayerPowerupCollision();
    }

    void CollisionSystem::handlePlayerProjectileEnemyCollision()
    {
        const auto &projectiles = m_entityManager.getProjectiles();
        const auto &enemies = m_entityManager.getEnemies();

        for (const auto &[projId, projEntity] : projectiles)
        {
            // Check if this is a player projectile
            const auto *projMetadata = m_entityManager.getEntityMetadata(projId);
            if (!projMetadata || projMetadata->type != ServerEntityType::PROJECTILE_PLAYER)
                continue;

            // Skip if projectile is already marked for destruction
            if (!projMetadata->isActive)
                continue;

            auto *projProjectile = m_registry.getComponent<ecs::Projectile>(projEntity);
            if (!projProjectile || projProjectile->pierce_remaining <= 0)
                continue;

            float projX, projY, projRadius;
            if (!getCollisionInfo(projEntity, projX, projY, projRadius))
                continue;

            // Check collision with all enemies
            for (const auto &[enemyId, enemyEntity] : enemies)
            {
                const auto *enemyMetadata = m_entityManager.getEntityMetadata(enemyId);
                if (!enemyMetadata || !enemyMetadata->isActive)
                    continue;

                float enemyX, enemyY, enemyRadius;
                if (!getCollisionInfo(enemyEntity, enemyX, enemyY, enemyRadius))
                    continue;

                float overlapDist;
                if (checkCircleCollision(projX, projY, projRadius, enemyX, enemyY, enemyRadius, &overlapDist))
                {
                    // Collision detected!
                    m_collisionCount++;

                    // Get projectile owner for score attribution
                    std::uint32_t ownerId = projMetadata->ownerId;

                    // Apply damage to enemy and award score if killed
                    applyDamageToEnemy(enemyId, projProjectile->damage, ownerId);

                    // Reduce pierce count
                    projProjectile->pierce_remaining--;

                    // Collision logged only for debugging (too verbose at INFO level)
                    // utl::Logger::log("CollisionSystem: Projectile " + std::to_string(projId) + " hit enemy " +
                    //                      std::to_string(enemyId) + " (pierce remaining: " +
                    //                      std::to_string(projProjectile->pierce_remaining) + ")");

                    // If projectile has no more pierce, mark for destruction
                    if (projProjectile->pierce_remaining <= 0)
                    {
                        m_entityManager.destroyProjectile(projId);
                        break; // Stop checking other enemies
                    }
                }
            }
        }
    }

    void CollisionSystem::handleEnemyProjectilePlayerCollision()
    {
        const auto &projectiles = m_entityManager.getProjectiles();
        const auto &players = m_entityManager.getPlayers();

        for (const auto &[projId, projEntity] : projectiles)
        {
            // Check if this is an enemy projectile
            const auto *projMetadata = m_entityManager.getEntityMetadata(projId);
            if (!projMetadata || projMetadata->type != ServerEntityType::PROJECTILE_ENEMY)
                continue;

            if (!projMetadata->isActive)
                continue;

            float projX, projY, projRadius;
            if (!getCollisionInfo(projEntity, projX, projY, projRadius))
                continue;

            auto *projProjectile = m_registry.getComponent<ecs::Projectile>(projEntity);
            if (!projProjectile)
                continue;

            // Check collision with all players
            for (const auto &[playerId, playerEntity] : players)
            {
                float playerX, playerY, playerRadius;
                if (!getCollisionInfo(playerEntity, playerX, playerY, playerRadius))
                    continue;

                if (checkCircleCollision(projX, projY, projRadius, playerX, playerY, playerRadius))
                {
                    // Collision detected!
                    m_collisionCount++;

                    // Apply damage to player
                    applyDamageToPlayer(playerId, projProjectile->damage);

                    // Destroy projectile
                    m_entityManager.destroyProjectile(projId);

                    // Collision logged only for debugging (too verbose at INFO level)
                    // utl::Logger::log("CollisionSystem: Enemy projectile " + std::to_string(projId) + " hit player " +
                    //                      std::to_string(playerId));

                    break; // Stop checking other players
                }
            }
        }
    }

    void CollisionSystem::handlePlayerEnemyCollision()
    {
        const auto &players = m_entityManager.getPlayers();
        const auto &enemies = m_entityManager.getEnemies();

        for (const auto &[playerId, playerEntity] : players)
        {
            float playerX, playerY, playerRadius;
            if (!getCollisionInfo(playerEntity, playerX, playerY, playerRadius))
                continue;

            // Check collision with all enemies
            for (const auto &[enemyId, enemyEntity] : enemies)
            {
                const auto *enemyMetadata = m_entityManager.getEntityMetadata(enemyId);
                if (!enemyMetadata || !enemyMetadata->isActive)
                    continue;

                float enemyX, enemyY, enemyRadius;
                if (!getCollisionInfo(enemyEntity, enemyX, enemyY, enemyRadius))
                    continue;

                if (checkCircleCollision(playerX, playerY, playerRadius, enemyX, enemyY, enemyRadius))
                {
                    // Collision detected!
                    m_collisionCount++;

                    // Get enemy damage
                    auto *enemy = m_registry.getComponent<ecs::Enemy>(enemyEntity);
                    float damage = enemy ? enemy->damage : 10.0f;

                    // Apply damage to player
                    applyDamageToPlayer(playerId, damage);

                    // Apply damage to enemy (ram damage) - no score for ram damage
                    applyDamageToEnemy(enemyId, 20.0f, 0);

                    // Collision logged only for debugging (too verbose at INFO level)
                    // utl::Logger::log("CollisionSystem: Player " + std::to_string(playerId) + " collided with enemy "
                    // +
                    //                      std::to_string(enemyId));
                }
            }
        }
    }

    void CollisionSystem::handlePlayerPowerupCollision()
    {
        // TODO: Implement when powerup collection is needed
        // For now, this is a placeholder
    }

    void CollisionSystem::applyDamageToEnemy(std::uint32_t enemyId, float damage, std::uint32_t attackerPlayerId)
    {
        ecs::Entity enemyEntity = m_entityManager.getEnemy(enemyId);
        if (enemyEntity == ecs::INVALID_ENTITY)
            return;

        auto *enemy = m_registry.getComponent<ecs::Enemy>(enemyEntity);
        if (!enemy)
            return;

        enemy->health -= damage;

        // Damage logged only for debugging (too verbose at INFO level)
        // utl::Logger::log("CollisionSystem: Enemy " + std::to_string(enemyId) + " took " + std::to_string(damage) +
        //                      " damage (health: " + std::to_string(enemy->health) + "/" +
        //                      std::to_string(enemy->max_health) + ")");

        // Check if enemy is dead
        if (enemy->health <= 0.0f)
        {
            utl::Logger::log("CollisionSystem: Enemy " + std::to_string(enemyId) + " destroyed", utl::LogLevel::INFO);

            // Award score to the player who killed the enemy
            if (attackerPlayerId > 0)
            {
                const auto *enemyMetadata = m_entityManager.getEntityMetadata(enemyId);
                int points = 0;

                // Different points based on enemy type
                if (enemyMetadata)
                {
                    switch (enemyMetadata->type)
                    {
                        case ServerEntityType::ENEMY_BASIC:
                            points = 100;
                            break;
                        case ServerEntityType::ENEMY_ADVANCED:
                            points = 250;
                            break;
                        case ServerEntityType::BOSS:
                            points = 1000;
                            break;
                        default:
                            points = 50;
                            break;
                    }
                }

                m_entityManager.addScore(attackerPlayerId, points);
            }

            m_entityManager.destroyEnemy(enemyId);
        }
    }

    void CollisionSystem::applyDamageToPlayer(std::uint32_t playerId, float damage)
    {
        ecs::Entity playerEntity = m_entityManager.getPlayerEntity(playerId);
        auto *health = m_registry.getComponent<ecs::Health>(playerEntity);

        if (!health)
        {
            utl::Logger::log("CollisionSystem: Player " + std::to_string(playerId) + " has no health component",
                             utl::LogLevel::WARNING);
            return;
        }

        health->current -= damage;

        utl::Logger::log("CollisionSystem: Player " + std::to_string(playerId) + " took " + std::to_string(damage) +
                             " damage (health: " + std::to_string(health->current) + "/" + std::to_string(health->max) +
                             ")",
                         utl::LogLevel::INFO);

        // Check if player is dead
        if (health->current <= 0.0f)
        {
            health->current = 0.0f;
            utl::Logger::log("CollisionSystem: Player " + std::to_string(playerId) + " died", utl::LogLevel::INFO);
            m_entityManager.markPlayerAsDead(playerId);
        }
    }

} // namespace gme
