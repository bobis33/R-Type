///
/// @file EnemyAISystem.cpp
/// @brief Implementation of enemy AI and behavior system
/// @namespace gme
///

#include "RTypeServer/Systems/EnemyAISystem.hpp"
#include "Utils/Logger.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace gme
{
    EnemyAISystem::EnemyAISystem(ecs::Registry &registry, EntityManager &entityManager)
        : m_registry(registry), m_entityManager(entityManager), m_aggressiveness(1.0f), m_shootChance(0.0f, 1.0f)
    {
        auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
        m_rng.seed(seed);

        utl::Logger::log("EnemyAISystem: Initialized", utl::LogLevel::INFO);
    }

    void EnemyAISystem::update(ecs::Registry &registry, float deltaTime)
    {
        (void)registry; // Use member registry instead
        const auto &enemies = m_entityManager.getEnemies();

        for (const auto &[enemyId, enemyEntity] : enemies)
        {
            const auto *metadata = m_entityManager.getEntityMetadata(enemyId);
            if (!metadata || !metadata->isActive)
                continue;

            // Update AI based on enemy type
            switch (metadata->type)
            {
                case ServerEntityType::ENEMY_BASIC:
                    updateBasicEnemyAI(enemyId, enemyEntity, deltaTime);
                    break;
                case ServerEntityType::ENEMY_ADVANCED:
                    updateAdvancedEnemyAI(enemyId, enemyEntity, deltaTime);
                    break;
                case ServerEntityType::BOSS:
                    updateBossAI(enemyId, enemyEntity, deltaTime);
                    break;
                default:
                    break;
            }

            // Clamp enemy to screen bounds
            clampToScreen(enemyEntity);
        }
    }

    void EnemyAISystem::updateBasicEnemyAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime)
    {
        auto *velocity = m_registry.getComponent<ecs::Velocity>(enemy);
        if (!velocity)
            return;

        // Basic enemies move straight left
        velocity->x = -200.0f;
        velocity->y = 0.0f;

        // Initialize movement timer if not present
        if (m_enemyMovementTimers.find(enemyId) == m_enemyMovementTimers.end())
        {
            m_enemyMovementTimers[enemyId] = 0.0f;
        }
        m_enemyMovementTimers[enemyId] += deltaTime;

        // Try to shoot occasionally
        tryShoot(enemyId, enemy, deltaTime);
    }

    void EnemyAISystem::updateAdvancedEnemyAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(enemy);
        if (!transform)
            return;

        // Initialize movement timer if not present
        if (m_enemyMovementTimers.find(enemyId) == m_enemyMovementTimers.end())
        {
            m_enemyMovementTimers[enemyId] = 0.0f;
        }
        m_enemyMovementTimers[enemyId] += deltaTime;

        // Advanced enemies use sine wave movement
        applySineWaveMovement(enemyId, enemy, deltaTime, 1.5f, 100.0f);

        // Check distance to nearest player
        float distanceToPlayer = getDistanceToNearestPlayer(transform->x, transform->y);

        // If player is close, become more aggressive
        if (distanceToPlayer < 400.0f)
        {
            applyAggressiveMovement(enemy, deltaTime, 100.0f);
        }

        // Try to shoot more frequently than basic enemies
        tryShoot(enemyId, enemy, deltaTime);
    }

    void EnemyAISystem::updateBossAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(enemy);
        auto *velocity = m_registry.getComponent<ecs::Velocity>(enemy);
        auto *enemyComp = m_registry.getComponent<ecs::Enemy>(enemy);

        if (!transform || !velocity || !enemyComp)
            return;

        // Initialize timers if not present
        if (m_enemyMovementTimers.find(enemyId) == m_enemyMovementTimers.end())
        {
            m_enemyMovementTimers[enemyId] = 0.0f;
        }
        if (m_bossSpreadTimers.find(enemyId) == m_bossSpreadTimers.end())
        {
            m_bossSpreadTimers[enemyId] = 0.0f;
        }
        m_enemyMovementTimers[enemyId] += deltaTime;
        m_bossSpreadTimers[enemyId] += deltaTime;

        // Boss has multiple phases based on health
        float healthPercent = enemyComp->health / enemyComp->max_health;

        if (healthPercent > 0.66f)
        {
            // Phase 1: Move slowly and shoot occasionally
            velocity->x = -30.0f;
            applySineWaveMovement(enemyId, enemy, deltaTime, 0.5f, 150.0f);

            // Reduce shoot cooldown
            enemyComp->shoot_cooldown = 1.5f;
        }
        else if (healthPercent > 0.33f)
        {
            // Phase 2: Move faster with zigzag pattern
            applyZigzagMovement(enemyId, enemy, deltaTime);

            // Shoot more frequently
            enemyComp->shoot_cooldown = 1.0f;
        }
        else
        {
            // Phase 3: Aggressive movement and rapid fire
            velocity->x = -50.0f;
            applyAggressiveMovement(enemy, deltaTime, 80.0f);

            // Rapid fire
            enemyComp->shoot_cooldown = 0.5f;
        }

        // Boss always tries to shoot
        tryShoot(enemyId, enemy, deltaTime);

        // Additional boss-specific behavior
        // Shoot multiple projectiles in a spread pattern when health is low
        if (healthPercent < 0.33f && m_bossSpreadTimers[enemyId] >= 1.0f)
        {
            // Spread pattern: shoot at angles
            for (int i = -1; i <= 1; i++)
            {
                if (i == 0)
                    continue; // Skip center (handled by normal shoot)

                float angle = i * 15.0f * (3.14159f / 180.0f); // Convert to radians
                float vx = -300.0f * std::cos(angle);
                float vy = -300.0f * std::sin(angle);

                m_entityManager.createEnemyProjectile(m_entityManager.getNetworkIdForEntity(enemy), transform->x,
                                                      transform->y, vx, vy);
            }
            m_bossSpreadTimers[enemyId] = 0.0f;
        }
    }

    void EnemyAISystem::applySineWaveMovement(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime,
                                              float frequency, float amplitude)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(enemy);
        auto *velocity = m_registry.getComponent<ecs::Velocity>(enemy);

        if (!transform || !velocity)
            return;

        // Base horizontal movement
        float baseVelocityX = velocity->x;
        if (baseVelocityX == 0.0f)
            baseVelocityX = -150.0f;

        // Use per-enemy timer instead of static variable
        float time = m_enemyMovementTimers[enemyId];
        float sineValue = std::sin(time * frequency);
        velocity->y = sineValue * amplitude;
        velocity->x = baseVelocityX;
    }

    void EnemyAISystem::applyAggressiveMovement(ecs::Entity enemy, float deltaTime, float speed)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(enemy);
        auto *velocity = m_registry.getComponent<ecs::Velocity>(enemy);

        if (!transform || !velocity)
            return;

        // Find nearest player
        ecs::Entity nearestPlayer = findNearestPlayer(transform->x, transform->y);
        if (nearestPlayer == ecs::INVALID_ENTITY)
        {
            // No player found, move left by default
            velocity->x = -speed;
            velocity->y = 0.0f;
            return;
        }

        auto *playerTransform = m_registry.getComponent<ecs::Transform>(nearestPlayer);
        if (!playerTransform)
            return;

        // Calculate direction to player
        float dx = playerTransform->x - transform->x;
        float dy = playerTransform->y - transform->y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0.01f)
        {
            // Normalize and apply speed
            velocity->x = (dx / distance) * speed * m_aggressiveness;
            velocity->y = (dy / distance) * speed * m_aggressiveness;
        }
    }

    void EnemyAISystem::applyZigzagMovement(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(enemy);
        auto *velocity = m_registry.getComponent<ecs::Velocity>(enemy);

        if (!transform || !velocity)
            return;

        // Use per-enemy timer instead of static variable
        float zigzagTime = m_enemyMovementTimers[enemyId];

        // Zigzag pattern: change direction every second
        float period = 1.5f;
        float phase = std::fmod(zigzagTime, period * 2.0f);

        velocity->x = -150.0f;
        velocity->y = (phase < period) ? 100.0f : -100.0f;
    }

    void EnemyAISystem::tryShoot(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime)
    {
        auto *enemyComp = m_registry.getComponent<ecs::Enemy>(enemy);
        auto *transform = m_registry.getComponent<ecs::Transform>(enemy);

        if (!enemyComp || !transform)
            return;

        // Update shot timer
        enemyComp->last_shot_time += deltaTime;

        // Check if can shoot
        if (!canShoot(enemy))
            return;

        // Random chance to shoot (to add variety)
        float randomValue = m_shootChance(m_rng);
        if (randomValue > 0.3f) // 70% chance to NOT shoot (30% chance to shoot)
            return;

        // Check if there's a player to shoot at
        float distanceToPlayer = getDistanceToNearestPlayer(transform->x, transform->y);
        if (distanceToPlayer > 1500.0f) // Don't shoot if no player is nearby
            return;

        // Shoot projectile towards the left (player direction)
        float projectileVx = -400.0f;
        float projectileVy = 0.0f;

        // For more intelligent enemies, shoot towards nearest player
        ecs::Entity nearestPlayer = findNearestPlayer(transform->x, transform->y);
        if (nearestPlayer != ecs::INVALID_ENTITY)
        {
            auto *playerTransform = m_registry.getComponent<ecs::Transform>(nearestPlayer);
            if (playerTransform)
            {
                float dx = playerTransform->x - transform->x;
                float dy = playerTransform->y - transform->y;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Only shoot towards the left (player direction) - don't shoot backwards
                if (distance > 0.01f && dx < 0.0f)
                {
                    // Normalize and apply projectile speed
                    float projectileSpeed = 500.0f;
                    projectileVx = (dx / distance) * projectileSpeed;
                    projectileVy = (dy / distance) * projectileSpeed;
                }
                else if (dx >= 0.0f)
                {
                    // Player is behind, don't shoot
                    return;
                }
            }
        }

        m_entityManager.createEnemyProjectile(enemyId, transform->x - 10.0f, transform->y, projectileVx, projectileVy);

        // Reset shot timer
        enemyComp->last_shot_time = 0.0f;

        // Shooting logged only for debugging (too verbose at INFO level)
        // utl::Logger::log("EnemyAISystem: Enemy " + std::to_string(enemyId) + " fired projectile");
    }

    bool EnemyAISystem::canShoot(ecs::Entity enemy) const
    {
        auto *enemyComp = m_registry.getComponent<ecs::Enemy>(enemy);
        if (!enemyComp)
            return false;

        return enemyComp->last_shot_time >= enemyComp->shoot_cooldown;
    }

    ecs::Entity EnemyAISystem::findNearestPlayer(float x, float y) const
    {
        const auto &players = m_entityManager.getPlayers();

        ecs::Entity nearestPlayer = ecs::INVALID_ENTITY;
        float nearestDistance = std::numeric_limits<float>::max();

        for (const auto &[playerId, playerEntity] : players)
        {
            auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);
            if (!transform)
                continue;

            float dx = transform->x - x;
            float dy = transform->y - y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestPlayer = playerEntity;
            }
        }

        return nearestPlayer;
    }

    float EnemyAISystem::getDistanceToNearestPlayer(float x, float y) const
    {
        const auto &players = m_entityManager.getPlayers();

        float nearestDistance = std::numeric_limits<float>::max();

        for (const auto &[playerId, playerEntity] : players)
        {
            auto *transform = m_registry.getComponent<ecs::Transform>(playerEntity);
            if (!transform)
                continue;

            float dx = transform->x - x;
            float dy = transform->y - y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < nearestDistance)
            {
                nearestDistance = distance;
            }
        }

        return nearestDistance;
    }

    void EnemyAISystem::clampToScreen(ecs::Entity enemy)
    {
        auto *transform = m_registry.getComponent<ecs::Transform>(enemy);
        if (!transform)
            return;

        const float MARGIN = 50.0f;

        // Clamp to screen bounds with margin
        transform->x = std::max(-MARGIN, std::min(transform->x, utl::GameConfig::Server::SCREEN_WIDTH + MARGIN));
        transform->y = std::max(MARGIN, std::min(transform->y, utl::GameConfig::Server::SCREEN_HEIGHT - MARGIN));
    }

} // namespace gme
