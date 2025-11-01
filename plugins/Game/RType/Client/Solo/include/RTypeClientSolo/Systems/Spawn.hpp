///
/// @file Spawn.hpp
/// @brief This file contains the spawn system definitions
/// @namespace gme
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Common.hpp"
#include "Utils/HitboxUtils.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

namespace gme
{

    class SpawnSystem final : public ecs::ASystem
    {
        public:
            explicit SpawnSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~SpawnSystem() override = default;

            SpawnSystem(const SpawnSystem &) = delete;
            SpawnSystem &operator=(const SpawnSystem &) = delete;
            SpawnSystem(SpawnSystem &&) = delete;
            SpawnSystem &operator=(SpawnSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(const bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                m_enemySpawnTimer += dt;
                m_waveTimer += dt;

                if (m_enemySpawnTimer >= utl::GameConfig::Enemy::Easy::SPAWN_RATE)
                {
                    spawnEnemy(registry);
                    m_enemySpawnTimer = 0.0f;
                }

                if (m_waveTimer >= 10.0f)
                {
                    spawnWave(registry);
                    m_waveTimer = 0.0f;
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            float m_enemySpawnTimer = 0.0F;
            float m_waveTimer = 0.0F;

            static void spawnEnemy(ecs::Registry &registry)
            {
                float x = utl::GameConfig::Screen::SPAWN_X;
                float y =
                    utl::GameConfig::Screen::MIN_Y +
                    (std::rand() % static_cast<int>(utl::GameConfig::Screen::MAX_Y - utl::GameConfig::Screen::MIN_Y));

                auto [offsetX, offsetY] = utl::calculateHitboxOffsetsRelative(
                    x, y, utl::GameConfig::Enemy::Easy::SPRITE_WIDTH, utl::GameConfig::Enemy::Easy::SPRITE_HEIGHT,
                    utl::GameConfig::Enemy::Easy::SCALE);

                registry.createEntity()
                    .with<ecs::Transform>("enemy_transform", x, y, 0.0f)
                    .with<ecs::Velocity>("enemy_velocity", -utl::GameConfig::Enemy::Easy::SPEED, 0.0f)
                    .with<ecs::Rect>("enemy_rect", 0.0f, 0.0f,
                                     static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                     static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_HEIGHT))
                    .with<ecs::Scale>("enemy_scale", utl::GameConfig::Enemy::Easy::SCALE,
                                      utl::GameConfig::Enemy::Easy::SCALE)
                    .with<ecs::Texture>("enemy_texture", utl::Path::Texture::TEXTURE_ENEMY_EASY)
                    .with<ecs::Animation>("enemy_animation", 0, utl::GameConfig::Enemy::Easy::ANIMATION_FRAMES,
                                          utl::GameConfig::Enemy::Easy::ANIMATION_DURATION, 0.0f,
                                          static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                          static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_HEIGHT),
                                          static_cast<int>(utl::GameConfig::Enemy::Easy::FRAMES_PER_ROW))
                    .with<ecs::Enemy>("enemy", utl::GameConfig::Enemy::Easy::HEALTH,
                                      utl::GameConfig::Enemy::Easy::HEALTH, utl::GameConfig::Enemy::Easy::DAMAGE,
                                      utl::GameConfig::Enemy::Easy::SPEED, 0.0f,
                                      utl::GameConfig::Enemy::Easy::SHOOT_COOLDOWN)
                    .with<ecs::Hitbox>("enemy_hitbox", utl::GameConfig::Hitbox::ENEMY_RADIUS, offsetX, offsetY)
                    .build();
            }

            static void spawnWave(ecs::Registry &registry)
            {
                const int waveSize = 5 + (std::rand() % 4);

                for (int i = 0; i < waveSize; ++i)
                {
                    float x = utl::GameConfig::Screen::SPAWN_X + (i * 100.0f);
                    float y = utl::GameConfig::Screen::MIN_Y +
                              (std::rand() %
                               static_cast<int>(utl::GameConfig::Screen::MAX_Y - utl::GameConfig::Screen::MIN_Y));
                    auto [offsetX, offsetY] = utl::calculateHitboxOffsetsRelative(
                        x, y, utl::GameConfig::Enemy::Easy::SPRITE_WIDTH, utl::GameConfig::Enemy::Easy::SPRITE_HEIGHT,
                        utl::GameConfig::Enemy::Easy::SCALE);

                    registry.createEntity()
                        .with<ecs::Transform>("enemy_wave_transform", x, y, 0.0f)
                        .with<ecs::Velocity>("enemy_wave_velocity", -utl::GameConfig::Enemy::Easy::SPEED, 0.0f)
                        .with<ecs::Rect>("enemy_wave_rect", 0.0f, 0.0f,
                                         static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                         static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_HEIGHT))
                        .with<ecs::Scale>("enemy_wave_scale", utl::GameConfig::Enemy::Easy::SCALE,
                                          utl::GameConfig::Enemy::Easy::SCALE)
                        .with<ecs::Texture>("enemy_wave_texture", utl::Path::Texture::TEXTURE_ENEMY_EASY)
                        .with<ecs::Animation>("enemy_wave_animation", 0,
                                              static_cast<int>(utl::GameConfig::Enemy::Easy::ANIMATION_FRAMES),
                                              utl::GameConfig::Enemy::Easy::ANIMATION_DURATION, 0.0f,
                                              static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_WIDTH),
                                              static_cast<int>(utl::GameConfig::Enemy::Easy::SPRITE_HEIGHT),
                                              static_cast<int>(utl::GameConfig::Enemy::Easy::FRAMES_PER_ROW))
                        .with<ecs::Enemy>("enemy_wave", utl::GameConfig::Enemy::Easy::HEALTH,
                                          utl::GameConfig::Enemy::Easy::HEALTH, utl::GameConfig::Enemy::Easy::DAMAGE,
                                          utl::GameConfig::Enemy::Easy::SPEED, 0.0f,
                                          utl::GameConfig::Enemy::Easy::SHOOT_COOLDOWN)
                        .with<ecs::Hitbox>("enemy_wave_hitbox", utl::GameConfig::Hitbox::ENEMY_RADIUS, offsetX, offsetY)
                        .build();
                }
            }
    }; // class SpawnSystem
} // namespace gme
