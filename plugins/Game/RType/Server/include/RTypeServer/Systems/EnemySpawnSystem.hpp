///
/// @file EnemySpawnSystem.hpp
/// @brief Enemy spawning system for server
/// @namespace gme
///

#pragma once

#include "ECS/Interfaces/ISystems.hpp"
#include "RTypeServer/EntityManager.hpp"
#include <random>
#include <vector>

namespace gme
{
    ///
    /// @brief Spawn wave configuration
    ///
    struct SpawnWave
    {
            float spawnTime; // Time from level start
            int basicEnemyCount;
            int advancedEnemyCount;
            bool bossSpawn;
            float spawnInterval; // Time between enemy spawns in wave
            int spawnedCount;    // Track how many have spawned
    };

    ///
    /// @class EnemySpawnSystem
    /// @brief Manages enemy spawning based on level progression
    /// @namespace gme
    ///
    class EnemySpawnSystem final : public ecs::ASystem
    {
        public:
            explicit EnemySpawnSystem(EntityManager &entityManager);
            ~EnemySpawnSystem() override = default;

            EnemySpawnSystem(const EnemySpawnSystem &) = delete;
            EnemySpawnSystem &operator=(const EnemySpawnSystem &) = delete;
            EnemySpawnSystem(EnemySpawnSystem &&) = delete;
            EnemySpawnSystem &operator=(EnemySpawnSystem &&) = delete;

            void update(ecs::Registry &registry, float deltaTime) override;
            void reset();

            // Configuration
            void setEnabled(bool enabled) { m_enabled = enabled; }
            bool isEnabled() const { return m_enabled; }

            // Statistics
            size_t getTotalEnemiesSpawned() const { return m_totalEnemiesSpawned; }
            size_t getCurrentWaveIndex() const { return m_currentWaveIndex; }
            float getLevelTime() const { return m_levelTime; }

        private:
            EntityManager &m_entityManager;
            float m_levelTime;
            size_t m_currentWaveIndex;
            float m_waveSpawnTimer;
            bool m_enabled;
            size_t m_totalEnemiesSpawned;

            std::vector<SpawnWave> m_waves;
            std::mt19937 m_rng;
            std::uniform_real_distribution<float> m_yDistribution;

            void initializeWaves();
            void processWave(SpawnWave &wave, float deltaTime);
            float getRandomY();
            float getSpawnX() const { return 2000.0f; } // Right side of screen
    };

} // namespace gme
