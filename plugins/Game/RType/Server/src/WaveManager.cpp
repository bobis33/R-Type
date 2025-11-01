/// @file WaveManager.cpp
/// @brief Implementation of wave-based enemy spawning system
/// @namespace gme
///

#include "RTypeServer/WaveManager.hpp"
#include <algorithm>
#include <iostream>

namespace gme
{

    WaveManager::WaveManager(EntityManager &entityManager)
        : m_entityManager(entityManager), m_currentWaveIndex(-1), m_waveTimer(0.0f), m_spawnTimer(0.0f),
          m_active(false), m_completed(false), m_waveInProgress(false), m_initialDelayPassed(false),
          m_initialDelayTimer(0.0f)
    {
    }

    void WaveManager::update(ecs::Registry &registry, const float dt, const int screenWidth)
    {
        if (!m_active || m_completed)
        {
            return;
        }

        // Handle initial delay (sync with client stage spawn)
        if (!m_initialDelayPassed)
        {
            m_initialDelayTimer += dt;
            if (m_initialDelayTimer >= INITIAL_DELAY)
            {
                m_initialDelayPassed = true;
                std::cout << "[WaveManager] Initial delay complete, waves starting!" << std::endl;
            }
            else
            {
                return; // Wait for initial delay
            }
        }

        // Check if we need to start the first wave
        if (m_currentWaveIndex < 0)
        {
            startNextWave();
            return;
        }

        // Update wave timer
        m_waveTimer += dt;
        m_spawnTimer += dt;

        // Process enemy spawns for current wave
        if (m_waveInProgress && m_currentWaveIndex < static_cast<int>(m_waves.size()))
        {
            processSpawns(dt, screenWidth);

            const Wave &currentWave = m_waves[m_currentWaveIndex];

            // Check if we should advance to next wave
            bool shouldAdvance = false;

            if (currentWave.waitForClear)
            {
                // Wait for all enemies to be cleared
                if (isWaveCleared())
                {
                    shouldAdvance = true;
                }
            }
            else
            {
                // Advance after duration expires
                if (m_waveTimer >= currentWave.duration)
                {
                    shouldAdvance = true;
                }
            }

            if (shouldAdvance)
            {
                // Call completion callback if exists
                if (currentWave.onComplete)
                {
                    currentWave.onComplete();
                }

                std::cout << "[WaveManager] Wave " << (m_currentWaveIndex + 1) << " completed!" << std::endl;

                // Move to next wave
                if (m_currentWaveIndex + 1 < static_cast<int>(m_waves.size()))
                {
                    startNextWave();
                }
                else
                {
                    // All waves completed
                    m_completed = true;
                    m_waveInProgress = false;
                    std::cout << "[WaveManager] All waves completed!" << std::endl;
                }
            }
        }
    }

    void WaveManager::start()
    {
        m_active = true;
        m_currentWaveIndex = -1;
        m_waveTimer = 0.0f;
        m_spawnTimer = 0.0f;
        m_completed = false;
        m_waveInProgress = false;
        m_initialDelayPassed = false;
        m_initialDelayTimer = 0.0f;
        std::cout << "[WaveManager] Wave system started with " << m_waves.size()
                  << " waves (initial delay: " << INITIAL_DELAY << "s)" << std::endl;
    }

    void WaveManager::reset()
    {
        m_active = false;
        m_currentWaveIndex = -1;
        m_waveTimer = 0.0f;
        m_spawnTimer = 0.0f;
        m_completed = false;
        m_waveInProgress = false;
        m_initialDelayPassed = false;
        m_initialDelayTimer = 0.0f;
        m_enemiesSpawned.clear();
        std::cout << "[WaveManager] Wave system reset" << std::endl;
    }

    void WaveManager::addWave(const Wave &wave) { m_waves.push_back(wave); }

    void WaveManager::clearWaves()
    {
        m_waves.clear();
        reset();
    }

    void WaveManager::setupDefaultWaves()
    {
        clearWaves();
        createWave1();
        createWave2();
        createWave3();
        createWave4();
        createBossWave();
        std::cout << "[WaveManager] Default waves created (" << m_waves.size() << " waves)" << std::endl;
    }

    void WaveManager::startNextWave()
    {
        m_currentWaveIndex++;

        if (m_currentWaveIndex >= static_cast<int>(m_waves.size()))
        {
            m_completed = true;
            m_waveInProgress = false;
            return;
        }

        const Wave &wave = m_waves[m_currentWaveIndex];

        m_waveTimer = 0.0f;
        m_spawnTimer = 0.0f;
        m_waveInProgress = true;
        m_enemiesSpawned.clear();
        m_enemiesSpawned.resize(wave.enemies.size(), false);

        std::cout << "[WaveManager] Starting wave " << (m_currentWaveIndex + 1) << " with " << wave.enemies.size()
                  << " enemies" << std::endl;
    }

    bool WaveManager::isWaveCleared() const
    {
        // Check if all spawned enemies are dead
        const auto &enemies = m_entityManager.getEnemies();
        return enemies.empty();
    }

    void WaveManager::processSpawns(const float dt, const int screenWidth)
    {
        if (m_currentWaveIndex < 0 || m_currentWaveIndex >= static_cast<int>(m_waves.size()))
        {
            return;
        }

        const Wave &currentWave = m_waves[m_currentWaveIndex];

        // Check each enemy spawn in the current wave
        for (size_t i = 0; i < currentWave.enemies.size(); ++i)
        {
            if (m_enemiesSpawned[i])
            {
                continue; // Already spawned
            }

            const EnemySpawn &spawn = currentWave.enemies[i];

            // Check if it's time to spawn this enemy
            if (m_spawnTimer >= spawn.spawnDelay)
            {
                // Calculate spawn position (right side of screen + offset)
                float spawnX = static_cast<float>(screenWidth) + spawn.x;
                float spawnY = spawn.y;

                // Create the enemy based on type
                switch (spawn.type)
                {
                    case ServerEntityType::ENEMY_BASIC:
                        m_entityManager.createBasicEnemy(spawnX, spawnY, spawn.health);
                        std::cout << "[WaveManager] Spawned Basic Enemy at (" << spawnX << ", " << spawnY << ")"
                                  << std::endl;
                        break;

                    case ServerEntityType::ENEMY_ADVANCED:
                        m_entityManager.createAdvancedEnemy(spawnX, spawnY, spawn.health);
                        std::cout << "[WaveManager] Spawned Advanced Enemy at (" << spawnX << ", " << spawnY << ")"
                                  << std::endl;
                        break;

                    case ServerEntityType::BOSS:
                        m_entityManager.createBoss(spawnX, spawnY, spawn.health);
                        std::cout << "[WaveManager] Spawned Boss at (" << spawnX << ", " << spawnY << ")" << std::endl;
                        break;

                    default:
                        std::cerr << "[WaveManager] Unknown enemy type in spawn!" << std::endl;
                        break;
                }

                m_enemiesSpawned[i] = true;
            }
        }
    }

    // ==================== Default Wave Creation ====================

    void WaveManager::createWave1()
    {
        Wave wave;
        wave.waveNumber = 1;
        wave.duration = 30.0f;
        wave.waitForClear = false; // Continue after 30s regardless

        // First group - 3 Basic enemies in vertical line
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 150.0f, 50.0f, 2.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 300.0f, 50.0f, 4.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 450.0f, 50.0f, 6.0f});

        // Second group - 2 Basic enemies from sides
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 80.0f, 200.0f, 50.0f, 12.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 80.0f, 400.0f, 50.0f, 12.0f});

        // Third group - 3 Basic enemies in formation
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 100.0f, 250.0f, 50.0f, 20.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 60.0f, 300.0f, 50.0f, 21.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 100.0f, 350.0f, 50.0f, 22.0f});

        m_waves.push_back(wave);
    }

    void WaveManager::createWave2()
    {
        Wave wave;
        wave.waveNumber = 2;
        wave.duration = 30.0f;
        wave.waitForClear = false; // Continue after 30s regardless

        // First wave - Advanced enemy with Basic support
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 80.0f, 300.0f, 100.0f, 2.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 40.0f, 200.0f, 60.0f, 3.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 40.0f, 400.0f, 60.0f, 3.0f});

        // Second wave - V formation of Basic enemies
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 100.0f, 200.0f, 60.0f, 10.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 60.0f, 250.0f, 60.0f, 10.5f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 20.0f, 300.0f, 60.0f, 11.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 60.0f, 350.0f, 60.0f, 11.5f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 100.0f, 400.0f, 60.0f, 12.0f});

        // Third wave - Two Advanced enemies
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 90.0f, 250.0f, 100.0f, 20.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 90.0f, 350.0f, 100.0f, 22.0f});

        m_waves.push_back(wave);
    }

    void WaveManager::createWave3()
    {
        Wave wave;
        wave.waveNumber = 3;
        wave.duration = 30.0f;
        wave.waitForClear = false; // Continue after 30s

        // First assault - Multiple Basic enemies
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 150.0f, 70.0f, 1.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 300.0f, 70.0f, 1.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 450.0f, 70.0f, 1.0f});

        // Second assault - Advanced enemies with escort
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 100.0f, 200.0f, 120.0f, 8.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 180.0f, 60.0f, 9.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 100.0f, 400.0f, 120.0f, 10.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 420.0f, 60.0f, 11.0f});

        // Final assault - Mini-boss wave
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 120.0f, 250.0f, 150.0f, 18.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 120.0f, 350.0f, 150.0f, 18.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 70.0f, 200.0f, 70.0f, 20.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 70.0f, 300.0f, 70.0f, 20.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 70.0f, 400.0f, 70.0f, 20.0f});

        m_waves.push_back(wave);
    }

    void WaveManager::createWave4()
    {
        Wave wave;
        wave.waveNumber = 4;
        wave.duration = 30.0f;
        wave.waitForClear = false; // Continue after 30s

        // Intense pre-boss wave with multiple formations
        // First formation - Top and bottom pincer
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 100.0f, 150.0f, 130.0f, 1.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 180.0f, 70.0f, 2.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 120.0f, 70.0f, 2.0f});

        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 100.0f, 450.0f, 130.0f, 1.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 420.0f, 70.0f, 2.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 480.0f, 70.0f, 2.0f});

        // Second formation - Center reinforcements
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 80.0f, 250.0f, 70.0f, 10.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 80.0f, 300.0f, 70.0f, 10.5f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 80.0f, 350.0f, 70.0f, 11.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 120.0f, 300.0f, 150.0f, 12.0f});

        // Third formation - Final assault before boss
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 130.0f, 200.0f, 150.0f, 20.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 130.0f, 400.0f, 150.0f, 20.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 70.0f, 250.0f, 70.0f, 22.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 70.0f, 300.0f, 70.0f, 22.5f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 70.0f, 350.0f, 70.0f, 23.0f});

        m_waves.push_back(wave);
    }

    void WaveManager::createBossWave()
    {
        Wave wave;
        wave.waveNumber = 5;
        wave.duration = 60.0f;
        wave.waitForClear = true; // Must kill boss to win

        // Boss spawn with initial escort
        wave.enemies.push_back({ServerEntityType::BOSS, 150.0f, 300.0f, 1000.0f, 3.0f});

        // Initial escort
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 80.0f, 200.0f, 60.0f, 4.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 80.0f, 400.0f, 60.0f, 4.0f});

        // First wave of reinforcements (T+15s)
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 100.0f, 250.0f, 100.0f, 15.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 100.0f, 350.0f, 100.0f, 15.0f});

        // Second wave of reinforcements (T+25s)
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 60.0f, 180.0f, 60.0f, 25.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 60.0f, 250.0f, 60.0f, 25.5f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 60.0f, 350.0f, 60.0f, 26.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 60.0f, 420.0f, 60.0f, 26.5f});

        // Third wave of reinforcements (T+40s) - desperation attack
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 120.0f, 200.0f, 120.0f, 40.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 120.0f, 300.0f, 120.0f, 40.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_ADVANCED, 120.0f, 400.0f, 120.0f, 40.0f});

        // Final reinforcements (T+50s) - last stand
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 150.0f, 70.0f, 50.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 225.0f, 70.0f, 50.5f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 300.0f, 70.0f, 51.0f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 375.0f, 70.0f, 51.5f});
        wave.enemies.push_back({ServerEntityType::ENEMY_BASIC, 50.0f, 450.0f, 70.0f, 52.0f});

        m_waves.push_back(wave);
    }

} // namespace gme
