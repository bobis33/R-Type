///
/// @file WaveManager.hpp
/// @brief Wave-based enemy spawning management system for R-Type server
/// @details This file contains the WaveManager class which provides a comprehensive wave-based
///          enemy spawning system for the R-Type game server. The manager handles timing,
///          sequencing, and spawning of enemies in predefined waves with configurable patterns.
///          It supports multiple wave types, spawn delays, wave completion detection, and
///          dynamic difficulty progression throughout the game.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "ECS/Registry.hpp"
#include "RTypeServer/EntityManager.hpp"
#include <functional>
#include <vector>

namespace gme
{
    ///
    /// @struct EnemySpawn
    /// @brief Definition of a single enemy spawn within a wave
    /// @details Specifies all parameters needed to spawn an enemy including type, position,
    ///          health, and timing. Multiple EnemySpawn instances make up a complete wave.
    ///
    struct EnemySpawn
    {
            ServerEntityType type; ///< Type of enemy to spawn (ENEMY_BASIC, ENEMY_ADVANCED, or BOSS)
            float x;               ///< Spawn X position (typically off-screen right)
            float y;               ///< Spawn Y position (vertical placement on screen)
            float health;          ///< Initial health points for this enemy
            float spawnDelay;      ///< Delay in seconds from wave start before spawning this enemy
    };

    ///
    /// @struct Wave
    /// @brief Complete wave configuration containing multiple enemy spawns
    /// @details Defines a complete enemy wave including all enemies to spawn, timing parameters,
    ///          completion conditions, and optional callbacks. Waves can either advance based on
    ///          time duration or wait for all enemies to be eliminated.
    ///
    struct Wave
    {
            int waveNumber;                   ///< Wave sequence number (1-based)
            std::vector<EnemySpawn> enemies;  ///< List of all enemies to spawn in this wave
            float duration;                   ///< Maximum duration in seconds before next wave (if not waitForClear)
            bool waitForClear;                ///< If true, next wave starts only when all enemies are destroyed
            std::function<void()> onComplete; ///< Optional callback function invoked when wave completes
    };

    ///
    /// @class WaveManager
    /// @brief Manages wave-based enemy spawning with configurable patterns and timing
    /// @details This class provides a complete wave management system for the R-Type server.
    ///          Features include:
    ///          - Sequential wave progression (5 waves: 4 standard waves + 1 boss wave)
    ///          - Time-based or clear-based wave advancement
    ///          - Per-enemy spawn delays within waves
    ///          - Initial delay synchronization with client stage spawning
    ///          - Wave completion detection
    ///          - Customizable wave patterns
    ///          - Default wave configurations for standard gameplay
    ///
    /// The manager maintains internal state for current wave, spawn timers, and completion
    /// status. It coordinates with the EntityManager to spawn enemies at appropriate times
    /// and ensures proper game progression through all waves.
    ///
    /// @namespace gme
    ///
    class WaveManager
    {
        public:
            ///
            /// @brief Constructor
            /// @param entityManager Reference to entity manager for spawning enemies
            /// @details Initializes the wave manager with default wave configurations
            ///
            explicit WaveManager(EntityManager &entityManager);

            ///
            /// @brief Destructor
            ///
            ~WaveManager() = default;

            /// @brief Deleted copy constructor (non-copyable)
            WaveManager(const WaveManager &) = delete;
            /// @brief Deleted copy assignment operator (non-copyable)
            WaveManager &operator=(const WaveManager &) = delete;
            /// @brief Deleted move constructor (non-movable)
            WaveManager(WaveManager &&) = delete;
            /// @brief Deleted move assignment operator (non-movable)
            WaveManager &operator=(WaveManager &&) = delete;

            ///
            /// @brief Update wave manager logic (called each frame)
            /// @param registry ECS registry containing all game entities
            /// @param dt Delta time since last frame (in seconds)
            /// @param screenWidth Screen width for spawn X positioning
            /// @details Processes initial delay, advances wave timers, spawns enemies according
            ///          to wave configurations, and checks for wave completion conditions
            ///
            void update(ecs::Registry &registry, float dt, int screenWidth);

            ///
            /// @brief Check if initial delay has passed and waves can start
            /// @return True if initial delay has elapsed
            /// @details Initial delay synchronizes wave spawning with client stage initialization
            ///
            bool isReady() const { return m_initialDelayPassed; }

            ///
            /// @brief Start the wave system
            /// @details Activates wave processing and begins initial delay countdown
            ///
            void start();

            ///
            /// @brief Stop and reset the wave system to initial state
            /// @details Stops wave processing, resets all timers, and clears wave progress
            ///
            void reset();

            ///
            /// @brief Check if wave system is currently active
            /// @return True if wave system is running
            ///
            bool isActive() const { return m_active; }

            ///
            /// @brief Get current wave index
            /// @return Current wave index (0-based)
            /// @details Returns the index of the currently active or next wave
            ///
            int getCurrentWave() const { return m_currentWaveIndex; }

            ///
            /// @brief Get total number of configured waves
            /// @return Total wave count (default: 5)
            ///
            size_t getTotalWaves() const { return m_waves.size(); }

            ///
            /// @brief Check if all waves have been completed
            /// @return True if all waves are finished
            /// @details Returns true when the last wave has completed and no more waves remain
            ///
            bool isCompleted() const { return m_completed; }

            ///
            /// @brief Add a custom wave to the wave list
            /// @param wave Wave configuration to add
            /// @details Appends a wave to the end of the wave sequence
            ///
            void addWave(const Wave &wave);

            ///
            /// @brief Clear all configured waves
            /// @details Removes all waves from the manager. Use setupDefaultWaves() to restore defaults.
            ///
            void clearWaves();

            ///
            /// @brief Initialize with default wave patterns
            /// @details Creates 5 predefined waves: 4 standard waves (30s each) + 1 boss wave (60s)
            ///
            void setupDefaultWaves();

        private:
            EntityManager &m_entityManager;     ///< Reference to entity manager for spawning
            std::vector<Wave> m_waves;          ///< List of all configured waves
            int m_currentWaveIndex;             ///< Index of current/next wave (0-based)
            float m_waveTimer;                  ///< Timer for current wave duration
            float m_spawnTimer;                 ///< Timer for processing enemy spawns
            bool m_active;                      ///< Whether wave system is active
            bool m_completed;                   ///< Whether all waves are completed
            bool m_waveInProgress;              ///< Whether a wave is currently spawning
            bool m_initialDelayPassed;          ///< Whether initial delay has elapsed
            float m_initialDelayTimer;          ///< Timer for initial delay countdown
            std::vector<bool> m_enemiesSpawned; ///< Track which enemies in current wave have spawned

            static constexpr float INITIAL_DELAY = 0.5f; ///< Initial delay in seconds (synced with client)

            ///
            /// @brief Start the next wave in sequence
            /// @details Advances to next wave, resets timers, and initializes spawn tracking
            ///
            void startNextWave();

            ///
            /// @brief Check if current wave is cleared of all enemies
            /// @return True if no enemies remain from current wave
            /// @details Used for waves with waitForClear flag to determine wave completion
            ///
            bool isWaveCleared() const;

            ///
            /// @brief Process enemy spawns for current wave
            /// @param dt Delta time since last frame
            /// @param screenWidth Screen width for spawn positioning
            /// @details Checks spawn delays and creates enemies at appropriate times
            ///
            void processSpawns(float dt, int screenWidth);

            ///
            /// @brief Create wave 1 configuration (basic introduction wave)
            /// @details 30-second wave with basic enemies
            ///
            void createWave1();

            ///
            /// @brief Create wave 2 configuration (mixed enemies)
            /// @details 30-second wave with basic and advanced enemies
            ///
            void createWave2();

            ///
            /// @brief Create wave 3 configuration (increased difficulty)
            /// @details 30-second wave with more advanced enemies
            ///
            void createWave3();

            ///
            /// @brief Create wave 4 configuration (final standard wave)
            /// @details 30-second wave with challenging enemy patterns
            ///
            void createWave4();

            ///
            /// @brief Create boss wave configuration (final challenge)
            /// @details 60-second wave featuring the boss enemy
            ///
            void createBossWave();
    };

} // namespace gme
