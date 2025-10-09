///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "Client/GameConfig.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class SpawnSystem final : public eng::ISystem
    {
        public:
            explicit SpawnSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~SpawnSystem() override = default;

            SpawnSystem(const SpawnSystem &) = delete;
            SpawnSystem &operator=(const SpawnSystem &) = delete;
            SpawnSystem(SpawnSystem &&) = delete;
            SpawnSystem &operator=(SpawnSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                m_enemySpawnTimer += dt;
                m_asteroidSpawnTimer += dt;
                m_waveTimer += dt;

                if (m_enemySpawnTimer >= GameConfig::Enemy::Easy::SPAWN_RATE)
                {
                    spawnEnemy(registry);
                    m_enemySpawnTimer = 0.0f;
                }

                if (m_waveTimer >= 10.0f)
                {
                    spawnWave(registry);
                    m_waveTimer = 0.0f;
                }

                if (m_asteroidSpawnTimer >= GameConfig::Asteroid::Small::SPAWN_RATE)
                {
                    spawnAsteroid(registry, ecs::Asteroid::SMALL);
                    m_asteroidSpawnTimer = 0.0f;
                }
            }

        private:
            eng::IRenderer &m_renderer;
            float m_enemySpawnTimer = 0.0f;
            float m_asteroidSpawnTimer = 0.0f;
            float m_waveTimer = 0.0f;

            void spawnEnemy(ecs::Registry &registry)
            {
                float x = GameConfig::Screen::SPAWN_X;
                float y = static_cast<float>(
                    GameConfig::Screen::MIN_Y +
                    (std::rand() % static_cast<int>(GameConfig::Screen::MAX_Y - GameConfig::Screen::MIN_Y)));

                registry.createEntity()
                    .with<ecs::Transform>("enemy_transform", x, y, 0.0f)
                    .with<ecs::Velocity>("enemy_velocity", -GameConfig::Enemy::Easy::SPEED, 0.0f)
                    .with<ecs::Rect>("enemy_rect", 0.0f, 0.0f, static_cast<int>(GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                     static_cast<int>(GameConfig::Enemy::Easy::SPRITE_HEIGHT))
                    .with<ecs::Scale>("enemy_scale", GameConfig::Enemy::Easy::SCALE, GameConfig::Enemy::Easy::SCALE)
                    .with<ecs::Texture>("enemy_texture", Path::Texture::TEXTURE_ENEMY_EASY)
                    .with<ecs::Animation>("enemy_animation", 0, GameConfig::Enemy::Easy::ANIMATION_FRAMES,
                                          GameConfig::Enemy::Easy::ANIMATION_DURATION, 0.0f,
                                          static_cast<int>(GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                          static_cast<int>(GameConfig::Enemy::Easy::SPRITE_HEIGHT),
                                          static_cast<int>(GameConfig::Enemy::Easy::FRAMES_PER_ROW))
                    .with<ecs::Enemy>("enemy", GameConfig::Enemy::Easy::HEALTH, GameConfig::Enemy::Easy::HEALTH,
                                      GameConfig::Enemy::Easy::DAMAGE, GameConfig::Enemy::Easy::SPEED, 0.0f,
                                      GameConfig::Enemy::Easy::SHOOT_COOLDOWN)
                    .with<ecs::Hitbox>("enemy_hitbox", GameConfig::Hitbox::ENEMY_RADIUS)
                    .build();
            }

            void spawnAsteroid(ecs::Registry &registry, ecs::Asteroid::Size size)
            {
                float x = GameConfig::Screen::SPAWN_X;
                float y = static_cast<float>(
                    GameConfig::Screen::MIN_Y +
                    (std::rand() % static_cast<int>(GameConfig::Screen::MAX_Y - GameConfig::Screen::MIN_Y)));

                registry.createEntity()
                    .with<ecs::Transform>("asteroid_transform", x, y, 0.0f)
                    .with<ecs::Velocity>("asteroid_velocity", -GameConfig::Asteroid::Small::SPEED, 0.0f)
                    .with<ecs::Rect>("asteroid_rect", 0.0f, 0.0f,
                                     static_cast<int>(GameConfig::Asteroid::Small::SPRITE_WIDTH),
                                     static_cast<int>(GameConfig::Asteroid::Small::SPRITE_HEIGHT))
                    .with<ecs::Scale>("asteroid_scale", GameConfig::Asteroid::Small::SCALE,
                                      GameConfig::Asteroid::Small::SCALE)
                    .with<ecs::Texture>("asteroid_texture", Path::Texture::TEXTURE_ASTEROID)
                    .with<ecs::Animation>("asteroid_animation", 0, GameConfig::Asteroid::Small::ANIMATION_FRAMES,
                                          GameConfig::Asteroid::Small::ANIMATION_DURATION, 0.0f,
                                          static_cast<int>(GameConfig::Asteroid::Small::SPRITE_WIDTH),
                                          static_cast<int>(GameConfig::Asteroid::Small::SPRITE_HEIGHT),
                                          static_cast<int>(GameConfig::Asteroid::Small::FRAMES_PER_ROW))
                    .with<ecs::Asteroid>("asteroid", size, GameConfig::Asteroid::Small::ROTATION_SPEED,
                                         GameConfig::Asteroid::Small::HEALTH)
                    .with<ecs::Hitbox>("asteroid_hitbox", GameConfig::Hitbox::ASTEROID_SMALL_RADIUS)
                    .build();
            }

            void spawnWave(ecs::Registry &registry)
            {
                int waveSize = 5 + (std::rand() % 4);

                for (int i = 0; i < waveSize; ++i)
                {
                    float x = GameConfig::Screen::SPAWN_X + (i * 100.0f);
                    float y = static_cast<float>(
                        GameConfig::Screen::MIN_Y +
                        (std::rand() % static_cast<int>(GameConfig::Screen::MAX_Y - GameConfig::Screen::MIN_Y)));

                    registry.createEntity()
                        .with<ecs::Transform>("enemy_wave_transform", x, y, 0.0f)
                        .with<ecs::Velocity>("enemy_wave_velocity", -GameConfig::Enemy::Easy::SPEED, 0.0f)
                        .with<ecs::Rect>("enemy_wave_rect", 0.0f, 0.0f,
                                         static_cast<int>(GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                         static_cast<int>(GameConfig::Enemy::Easy::SPRITE_HEIGHT))
                        .with<ecs::Scale>("enemy_wave_scale", GameConfig::Enemy::Easy::SCALE,
                                          GameConfig::Enemy::Easy::SCALE)
                        .with<ecs::Texture>("enemy_wave_texture", Path::Texture::TEXTURE_ENEMY_EASY)
                        .with<ecs::Animation>("enemy_wave_animation", 0,
                                              static_cast<int>(GameConfig::Enemy::Easy::ANIMATION_FRAMES),
                                              GameConfig::Enemy::Easy::ANIMATION_DURATION, 0.0f,
                                              static_cast<int>(GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                              static_cast<int>(GameConfig::Enemy::Easy::SPRITE_HEIGHT),
                                              static_cast<int>(GameConfig::Enemy::Easy::FRAMES_PER_ROW))
                        .with<ecs::Enemy>("enemy_wave", GameConfig::Enemy::Easy::HEALTH,
                                          GameConfig::Enemy::Easy::HEALTH, GameConfig::Enemy::Easy::DAMAGE,
                                          GameConfig::Enemy::Easy::SPEED, 0.0f, GameConfig::Enemy::Easy::SHOOT_COOLDOWN)
                        .with<ecs::Hitbox>("enemy_wave_hitbox", GameConfig::Hitbox::ENEMY_RADIUS)
                        .build();
                }
            }
    }; // class SpawnSystem

} // namespace cli