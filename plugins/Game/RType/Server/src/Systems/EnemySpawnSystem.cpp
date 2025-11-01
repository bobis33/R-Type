///
/// @file EnemySpawnSystem.cpp
/// @brief Implementation of enemy spawning system
/// @namespace gme
///

#include "RTypeServer/Systems/EnemySpawnSystem.hpp"
#include "Utils/Logger.hpp"
#include <chrono>

namespace gme
{
    EnemySpawnSystem::EnemySpawnSystem(EntityManager &entityManager)
        : m_entityManager(entityManager), m_levelTime(0.0f), m_currentWaveIndex(0), m_waveSpawnTimer(0.0f),
          m_enabled(false), m_totalEnemiesSpawned(0), m_yDistribution(100.0f, 980.0f)
    {
        const auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
        m_rng.seed(seed);

        initializeWaves();

        utl::Logger::log("EnemySpawnSystem: Initialized with " + std::to_string(m_waves.size()) + " waves",
                         utl::LogLevel::INFO);
    }

    void EnemySpawnSystem::update(ecs::Registry &registry, float deltaTime)
    {
        (void)registry; // Use member entityManager instead
        if (!m_enabled)
            return;

        m_levelTime += deltaTime;
        m_waveSpawnTimer += deltaTime;

        // Check if we should process current wave
        if (m_currentWaveIndex < m_waves.size())
        {
            auto &wave = m_waves[m_currentWaveIndex];

            // Check if it's time to start this wave
            if (m_levelTime >= wave.spawnTime)
            {
                processWave(wave, deltaTime);

                // Check if wave is complete
                int totalToSpawn = wave.basicEnemyCount + wave.advancedEnemyCount + (wave.bossSpawn ? 1 : 0);
                if (wave.spawnedCount >= totalToSpawn)
                {
                    // Move to next wave
                    m_currentWaveIndex++;
                    m_waveSpawnTimer = 0.0f;

                    if (m_currentWaveIndex < m_waves.size())
                    {
                        utl::Logger::log("EnemySpawnSystem: Wave " + std::to_string(m_currentWaveIndex) +
                                             " complete. Moving to wave " + std::to_string(m_currentWaveIndex + 1),
                                         utl::LogLevel::INFO);
                    }
                    else
                    {
                        utl::Logger::log("EnemySpawnSystem: All waves complete!", utl::LogLevel::INFO);
                    }
                }
            }
        }
    }

    void EnemySpawnSystem::reset()
    {
        m_levelTime = 0.0f;
        m_currentWaveIndex = 0;
        m_waveSpawnTimer = 0.0f;
        m_totalEnemiesSpawned = 0;
        m_enabled = false;

        // Reset wave spawn counts
        for (auto &wave : m_waves)
        {
            wave.spawnedCount = 0;
        }

        utl::Logger::log("EnemySpawnSystem: Reset", utl::LogLevel::INFO);
    }

    void EnemySpawnSystem::initializeWaves()
    {
        m_waves.clear();

        // Wave 1: 5 basic enemies (10 seconds in, 1 enemy every 1 second)
        SpawnWave wave1;
        wave1.spawnTime = 10.0f;
        wave1.basicEnemyCount = 5;
        wave1.advancedEnemyCount = 0;
        wave1.bossSpawn = false;
        wave1.spawnInterval = 1.0f;
        wave1.spawnedCount = 0;
        m_waves.push_back(wave1);

        // Wave 2: 3 basic + 2 advanced (20 seconds in, 1 enemy every 1.5 seconds)
        SpawnWave wave2;
        wave2.spawnTime = 20.0f;
        wave2.basicEnemyCount = 3;
        wave2.advancedEnemyCount = 2;
        wave2.bossSpawn = false;
        wave2.spawnInterval = 1.5f;
        wave2.spawnedCount = 0;
        m_waves.push_back(wave2);

        // Wave 3: 2 basic + 4 advanced (35 seconds in, 1 enemy every 1.2 seconds)
        SpawnWave wave3;
        wave3.spawnTime = 35.0f;
        wave3.basicEnemyCount = 2;
        wave3.advancedEnemyCount = 4;
        wave3.bossSpawn = false;
        wave3.spawnInterval = 1.2f;
        wave3.spawnedCount = 0;
        m_waves.push_back(wave3);

        // Wave 4: 7 advanced enemies (55 seconds in, 1 enemy every 2 seconds)
        SpawnWave wave4;
        wave4.spawnTime = 55.0f;
        wave4.basicEnemyCount = 0;
        wave4.advancedEnemyCount = 7;
        wave4.bossSpawn = false;
        wave4.spawnInterval = 2.0f;
        wave4.spawnedCount = 0;
        m_waves.push_back(wave4);

        // Wave 5: Boss (80 seconds in)
        SpawnWave wave5;
        wave5.spawnTime = 80.0f;
        wave5.basicEnemyCount = 0;
        wave5.advancedEnemyCount = 0;
        wave5.bossSpawn = true;
        wave5.spawnInterval = 0.0f;
        wave5.spawnedCount = 0;
        m_waves.push_back(wave5);
    }

    void EnemySpawnSystem::processWave(SpawnWave &wave, float deltaTime)
    {
        (void)deltaTime; // Not used directly, but kept for consistency

        int totalToSpawn = wave.basicEnemyCount + wave.advancedEnemyCount + (wave.bossSpawn ? 1 : 0);

        // Check if we've spawned everything for this wave
        if (wave.spawnedCount >= totalToSpawn)
            return;

        // Check if enough time has passed since last spawn
        if (m_waveSpawnTimer < wave.spawnInterval)
            return;

        // Spawn next enemy
        float spawnX = getSpawnX();
        float spawnY = getRandomY();

        // Determine which type to spawn based on what's left
        if (wave.bossSpawn && wave.spawnedCount == 0)
        {
            // Spawn boss first if this is a boss wave
            m_entityManager.createBoss(spawnX, spawnY, 1000.0f);
            m_totalEnemiesSpawned++;
            wave.spawnedCount++;

            utl::Logger::log("EnemySpawnSystem: Spawned BOSS at (" + std::to_string(spawnX) + ", " +
                                 std::to_string(spawnY) + ")",
                             utl::LogLevel::INFO);
        }
        else if (wave.basicEnemyCount > 0 && wave.spawnedCount < wave.basicEnemyCount)
        {
            // Spawn basic enemy
            m_entityManager.createBasicEnemy(spawnX, spawnY, 50.0f);
            m_totalEnemiesSpawned++;
            wave.spawnedCount++;

            utl::Logger::log("EnemySpawnSystem: Spawned basic enemy at (" + std::to_string(spawnX) + ", " +
                                 std::to_string(spawnY) + ")",
                             utl::LogLevel::INFO);
        }
        else if (wave.advancedEnemyCount > 0)
        {
            // Spawn advanced enemy
            m_entityManager.createAdvancedEnemy(spawnX, spawnY, 100.0f);
            m_totalEnemiesSpawned++;
            wave.spawnedCount++;

            utl::Logger::log("EnemySpawnSystem: Spawned advanced enemy at (" + std::to_string(spawnX) + ", " +
                                 std::to_string(spawnY) + ")",
                             utl::LogLevel::INFO);
        }

        // Reset spawn timer
        m_waveSpawnTimer = 0.0f;
    }

    float EnemySpawnSystem::getRandomY()
    {
        // Return a random Y position within screen bounds
        // Screen height is 1080, so we spawn between 100 and 980 to leave margins
        return m_yDistribution(m_rng);
    }

} // namespace gme
