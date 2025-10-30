///
/// @file EnemyAISystem.hpp
/// @brief Enemy AI and behavior system for server
/// @namespace gme
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "RTypeServer/EntityManager.hpp"
#include <random>
#include <unordered_map>

namespace gme
{
    ///
    /// @brief Enemy behavior pattern types
    ///
    enum class EnemyBehavior : uint8_t
    {
        STRAIGHT = 0,   // Move straight left
        SINE_WAVE = 1,  // Sine wave pattern
        AGGRESSIVE = 2, // Follow player
        STATIONARY = 3, // Stay in place and shoot
        ZIGZAG = 4      // Zigzag pattern
    };

    ///
    /// @class EnemyAISystem
    /// @brief Manages enemy behavior, movement patterns, and shooting
    /// @namespace gme
    ///
    class EnemyAISystem final : public ecs::ASystem
    {
        public:
            explicit EnemyAISystem(ecs::Registry &registry, EntityManager &entityManager);
            ~EnemyAISystem() override = default;

            EnemyAISystem(const EnemyAISystem &) = delete;
            EnemyAISystem &operator=(const EnemyAISystem &) = delete;
            EnemyAISystem(EnemyAISystem &&) = delete;
            EnemyAISystem &operator=(EnemyAISystem &&) = delete;

            void update(ecs::Registry &registry, float deltaTime) override;

            // Configuration
            void setAggressiveness(float value) { m_aggressiveness = value; }
            float getAggressiveness() const { return m_aggressiveness; }

        private:
            ecs::Registry &m_registry;
            EntityManager &m_entityManager;

            float m_aggressiveness; // How aggressively enemies pursue players
            std::mt19937 m_rng;
            std::uniform_real_distribution<float> m_shootChance;

            // Per-entity timing data to avoid static variable synchronization
            std::unordered_map<std::uint32_t, float> m_enemyMovementTimers;
            std::unordered_map<std::uint32_t, float> m_bossSpreadTimers;

            // AI behaviors for different enemy types
            void updateBasicEnemyAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);
            void updateAdvancedEnemyAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);
            void updateBossAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);

            // Movement patterns (now takes enemyId for per-entity timing)
            void applySineWaveMovement(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime,
                                       float frequency = 2.0f, float amplitude = 50.0f);
            void applyAggressiveMovement(ecs::Entity enemy, float deltaTime, float speed = 150.0f);
            void applyZigzagMovement(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);

            // Shooting logic
            void tryShoot(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);
            bool canShoot(ecs::Entity enemy) const;

            // Utility
            ecs::Entity findNearestPlayer(float x, float y) const;
            float getDistanceToNearestPlayer(float x, float y) const;
            void clampToScreen(ecs::Entity enemy);
    };

} // namespace gme
