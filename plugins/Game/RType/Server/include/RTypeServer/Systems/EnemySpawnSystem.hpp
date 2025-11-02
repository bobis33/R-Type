///
/// @file EnemySpawnSystem.hpp
/// @brief Server-side enemy spawning system for R-Type game progression
/// @details This file contains the enemy spawning system that controls the timing and
///          placement of enemy waves throughout the game. The system manages spawn timing,
///          enemy type distribution, wave progression, and boss encounters based on level time.
///          All spawning is server-authoritative to ensure synchronized gameplay.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "ECS/Interfaces/ISystems.hpp"
#include "RTypeServer/EntityManager.hpp"
#include <random>
#include <vector>

namespace gme
{
    ///
    /// @struct SpawnWave
    /// @brief Configuration data for a single enemy spawn wave
    /// @details Defines when and how many enemies spawn during a specific wave.
    ///          Waves are triggered based on level time and can contain multiple enemy types
    ///          including bosses. Each wave spawns enemies gradually over time using the
    ///          spawn interval.
    ///
    struct SpawnWave
    {
            float spawnTime;        ///< Time from level start when wave begins (seconds)
            int basicEnemyCount;    ///< Number of basic enemies to spawn in this wave
            int advancedEnemyCount; ///< Number of advanced enemies to spawn in this wave
            bool bossSpawn;         ///< Whether this wave spawns a boss enemy
            float spawnInterval;    ///< Time between individual enemy spawns within wave (seconds)
            int spawnedCount;       ///< Number of enemies already spawned from this wave (internal counter)
    };

    ///
    /// @class EnemySpawnSystem
    /// @brief Server-side system that manages enemy wave spawning and level progression
    /// @details This ECS system controls the flow of enemy spawns throughout a game session.
    ///          Features include:
    ///          - Time-based wave triggering
    ///          - Multiple enemy types per wave (basic, advanced, boss)
    ///          - Gradual spawning with configurable intervals
    ///          - Random spawn position generation
    ///          - Wave progression tracking
    ///          - Spawn statistics for debugging
    ///
    /// The system maintains internal state for level time and wave progression,
    /// and spawns enemies at the right edge of the screen with random Y positions.
    /// All spawning is deterministic based on server time to ensure synchronized
    /// gameplay across all clients.
    ///
    /// @namespace gme
    ///
    class EnemySpawnSystem final : public ecs::ASystem
    {
        public:
            ///
            /// @brief Constructor
            /// @param entityManager Entity manager for creating enemy entities
            /// @details Initializes the spawn system and sets up initial wave configurations
            ///
            explicit EnemySpawnSystem(EntityManager &entityManager);

            ///
            /// @brief Destructor
            ///
            ~EnemySpawnSystem() override = default;

            /// @brief Deleted copy constructor (non-copyable)
            EnemySpawnSystem(const EnemySpawnSystem &) = delete;
            /// @brief Deleted copy assignment operator (non-copyable)
            EnemySpawnSystem &operator=(const EnemySpawnSystem &) = delete;
            /// @brief Deleted move constructor (non-movable)
            EnemySpawnSystem(EnemySpawnSystem &&) = delete;
            /// @brief Deleted move assignment operator (non-movable)
            EnemySpawnSystem &operator=(EnemySpawnSystem &&) = delete;

            ///
            /// @brief Update the spawn system (called each frame)
            /// @param registry ECS registry containing all entities
            /// @param deltaTime Time elapsed since last frame (in seconds)
            /// @details Advances level time, checks for wave triggers, and spawns enemies
            ///          according to wave configurations
            ///
            void update(ecs::Registry &registry, float deltaTime) override;

            ///
            /// @brief Reset the spawn system to initial state
            /// @details Resets level time, wave index, and spawn counters. Used when starting a new game.
            ///
            void reset();

            ///
            /// @brief Enable or disable enemy spawning
            /// @param enabled True to enable spawning, false to disable
            /// @details When disabled, no enemies will spawn regardless of wave timing
            ///
            void setEnabled(bool enabled) { m_enabled = enabled; }

            ///
            /// @brief Check if enemy spawning is currently enabled
            /// @return True if spawning is enabled
            ///
            bool isEnabled() const { return m_enabled; }

            ///
            /// @brief Get total number of enemies spawned since reset
            /// @return Total enemy spawn count
            /// @details Useful for statistics and debugging
            ///
            size_t getTotalEnemiesSpawned() const { return m_totalEnemiesSpawned; }

            ///
            /// @brief Get current wave index
            /// @return Index of the currently active or next wave
            /// @details Returns the index in the wave array, useful for level progression tracking
            ///
            size_t getCurrentWaveIndex() const { return m_currentWaveIndex; }

            ///
            /// @brief Get elapsed time since level start
            /// @return Level time in seconds
            /// @details Used for wave timing and level progression
            ///
            float getLevelTime() const { return m_levelTime; }

        private:
            EntityManager &m_entityManager; ///< Entity manager for spawning enemies
            float m_levelTime;              ///< Elapsed time since level start (seconds)
            size_t m_currentWaveIndex;      ///< Index of current/next wave to process
            float m_waveSpawnTimer;         ///< Timer for spawning enemies within current wave
            bool m_enabled;                 ///< Whether spawning is currently enabled
            size_t m_totalEnemiesSpawned;   ///< Total number of enemies spawned this session

            std::vector<SpawnWave> m_waves;                        ///< List of all spawn waves for the level
            std::mt19937 m_rng;                                    ///< Random number generator for spawn positions
            std::uniform_real_distribution<float> m_yDistribution; ///< Distribution for random Y positions

            ///
            /// @brief Initialize all spawn waves for the level
            /// @details Populates the m_waves vector with predefined wave configurations
            ///          including timing, enemy counts, and boss spawns
            ///
            void initializeWaves();

            ///
            /// @brief Process a single spawn wave
            /// @param wave Wave configuration to process
            /// @param deltaTime Time elapsed since last frame
            /// @details Handles the spawning of enemies from this wave according to spawn interval,
            ///          spawning enemies gradually over time
            ///
            void processWave(SpawnWave &wave, float deltaTime);

            ///
            /// @brief Generate random Y position for enemy spawn
            /// @return Random Y coordinate within valid screen bounds
            /// @details Uses uniform distribution to place enemies at various heights
            ///
            float getRandomY();

            ///
            /// @brief Get X position for enemy spawns (right side of screen)
            /// @return X coordinate for spawn position (2000.0f)
            /// @details Enemies spawn off-screen to the right and move left
            ///
            float getSpawnX() const { return 2000.0f; }
    };

} // namespace gme
