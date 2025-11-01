/// @file WaveManager.hpp
/// @brief Wave-based enemy spawning system for RTypeServer
/// @namespace gme
///

#pragma once

#include "ECS/Registry.hpp"
#include "RTypeServer/EntityManager.hpp"
#include <functional>
#include <vector>

namespace gme
{
    ///
    /// @brief Enemy spawn definition for a wave
    ///
    struct EnemySpawn
    {
            ServerEntityType type;
            float x;          // Spawn X position
            float y;          // Spawn Y position
            float health;     // Enemy health
            float spawnDelay; // Delay before spawning this enemy (in seconds from wave start)
    };

    ///
    /// @brief Wave definition containing multiple enemy spawns
    ///
    struct Wave
    {
            int waveNumber;
            std::vector<EnemySpawn> enemies;
            float duration;                   // How long until next wave if not cleared
            bool waitForClear;                // If true, wait until all enemies are dead
            std::function<void()> onComplete; // Optional callback when wave completes
    };

    ///
    /// @class WaveManager
    /// @brief Manages wave-based enemy spawning with configurable patterns
    /// @namespace gme
    ///
    class WaveManager
    {
        public:
            explicit WaveManager(EntityManager &entityManager);
            ~WaveManager() = default;

            WaveManager(const WaveManager &) = delete;
            WaveManager &operator=(const WaveManager &) = delete;
            WaveManager(WaveManager &&) = delete;
            WaveManager &operator=(WaveManager &&) = delete;

            /// @brief Update wave manager logic
            /// @param registry The ECS registry
            /// @param dt Delta time in seconds
            /// @param screenWidth Screen width for spawn positioning
            void update(ecs::Registry &registry, float dt, int screenWidth);

            /// @brief Check if initial delay has passed and waves can start
            bool isReady() const { return m_initialDelayPassed; }

            /// @brief Start the wave system
            void start();

            /// @brief Stop and reset the wave system
            void reset();

            /// @brief Check if wave system is active
            bool isActive() const { return m_active; }

            /// @brief Get current wave number
            int getCurrentWave() const { return m_currentWaveIndex; }

            /// @brief Get total number of waves
            size_t getTotalWaves() const { return m_waves.size(); }

            /// @brief Check if all waves are completed
            bool isCompleted() const { return m_completed; }

            /// @brief Add a custom wave
            void addWave(const Wave &wave);

            /// @brief Clear all waves
            void clearWaves();

            /// @brief Initialize with default wave patterns
            void setupDefaultWaves();

        private:
            EntityManager &m_entityManager;
            std::vector<Wave> m_waves;
            int m_currentWaveIndex;
            float m_waveTimer;
            float m_spawnTimer;
            bool m_active;
            bool m_completed;
            bool m_waveInProgress;
            bool m_initialDelayPassed;
            float m_initialDelayTimer;
            std::vector<bool> m_enemiesSpawned; // Track which enemies in current wave have spawned

            static constexpr float INITIAL_DELAY = 0.5f; // Sync with client STAGE_SPAWN_DELAY

            /// @brief Start the next wave
            void startNextWave();

            /// @brief Check if current wave is cleared
            bool isWaveCleared() const;

            /// @brief Process enemy spawns for current wave
            void processSpawns(float dt, int screenWidth);

            /// @brief Create default wave patterns (5 waves: 4x30s + boss 60s)
            void createWave1();
            void createWave2();
            void createWave3();
            void createWave4();
            void createBossWave();
    };

} // namespace gme
