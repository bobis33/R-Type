///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace gme
///

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "RTypeClientSolo/Common.hpp"
#include "RTypeClientSolo/GameConfig.hpp"

namespace gme
{

    class CollisionSystem final : public eng::ISystem
    {
        public:
            explicit CollisionSystem(const std::shared_ptr<eng::IRenderer> &renderer, bool &showDebug)
                : m_renderer(renderer), m_showDebug(showDebug)
            {
                m_enemyDeathAudioEntities.fill(ecs::INVALID_ENTITY);
            }
            ~CollisionSystem() override = default;

            CollisionSystem(const CollisionSystem &) = delete;
            CollisionSystem &operator=(const CollisionSystem &) = delete;
            CollisionSystem(CollisionSystem &&) = delete;
            CollisionSystem &operator=(CollisionSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                const bool hasPlayer = !registry.getAll<ecs::Player>().empty();

                std::optional<float> ceilingBottomY;
                std::optional<float> floorTopY;
                for (auto &[entity, tag] : registry.getAll<ecs::Ceiling>())
                {
                    const auto *t = registry.getComponent<ecs::Transform>(entity);
                    const auto *s = registry.getComponent<ecs::Scale>(entity);
                    const auto *scroll = registry.getComponent<ecs::Scrolling>(entity);
                    if (!t || !scroll)
                        continue;
                    const float scaledHeight = (s ? s->y : 1.0f) * scroll->original_height;
                    ceilingBottomY = t->y + scaledHeight;
                    break; // une seule bande suffit
                }
                for (auto &[entity, tag] : registry.getAll<ecs::Floor>())
                {
                    const auto *t = registry.getComponent<ecs::Transform>(entity);
                    if (!t)
                        continue;
                    floorTopY = t->y; // haut de la bande sol
                    break;
                }

                std::vector<ecs::Entity> projectilesToRemove;
                std::vector<ecs::Entity> enemiesToRemove;

                for (auto &[projectileEntity, projectile] : registry.getAll<ecs::Projectile>())
                {
                    auto *projectileTransform = registry.getComponent<ecs::Transform>(projectileEntity);
                    auto *projectileHitbox = registry.getComponent<ecs::Hitbox>(projectileEntity);
                    if (!projectileTransform || !projectileHitbox)
                        continue;

                    for (auto &[enemyEntity, enemy] : registry.getAll<ecs::Enemy>())
                    {
                        auto *enemyTransform = registry.getComponent<ecs::Transform>(enemyEntity);
                        auto *enemyHitbox = registry.getComponent<ecs::Hitbox>(enemyEntity);
                        if (!enemyTransform || !enemyHitbox)
                            continue;

                        if (checkCircularCollision(*projectileTransform, *projectileHitbox, *enemyTransform,
                                                   *enemyHitbox))
                        {
                            enemy.health -= projectile.damage;
                            if (projectile.type == ecs::Projectile::SUPERCHARGED && projectile.pierce_remaining > 1)
                            {
                                projectile.pierce_remaining -= 1; // continue sa course
                            }
                            else
                            {
                                projectilesToRemove.push_back(projectileEntity);
                            }

                            if (enemy.health <= 0.0f)
                            {
                                createExplosion(registry, enemyTransform->x, enemyTransform->y);
                                enemiesToRemove.push_back(enemyEntity);
                                for (auto &[scoreEntity, score] : registry.getAll<ecs::Score>())
                                {
                                    score.value += 100;
                                    break;
                                }
                                playEnemyDeathSound(registry);
                            }
                            break;
                        }
                    }
                }

                for (ecs::Entity entity : projectilesToRemove)
                {
                    removeProjectile(registry, entity);
                }
                for (ecs::Entity entity : enemiesToRemove)
                {
                    removeEnemy(registry, entity);
                }
                if (ceilingBottomY.has_value() || floorTopY.has_value())
                {
                    for (auto &[playerEntity, player] : registry.getAll<ecs::Player>())
                    {
                        auto *t = registry.getComponent<ecs::Transform>(playerEntity);
                        auto *hb = registry.getComponent<ecs::Hitbox>(playerEntity);
                        auto *vel = registry.getComponent<ecs::Velocity>(playerEntity);
                        if (!t || !hb)
                            continue;
                        float hitboxY = t->y + hb->offsetY;
                        if (ceilingBottomY.has_value() && (hitboxY - hb->radius < ceilingBottomY.value()))
                        {
                            t->y = ceilingBottomY.value() + hb->radius - hb->offsetY;
                            if (vel)
                                vel->y = std::max(0.0f, vel->y);
                        }
                        if (floorTopY.has_value() && (hitboxY + hb->radius > floorTopY.value()))
                        {
                            t->y = floorTopY.value() - hb->radius - hb->offsetY;
                            if (vel)
                                vel->y = std::min(0.0f, vel->y);
                        }
                    }
                    for (auto &[enemyEntity, enemy] : registry.getAll<ecs::Enemy>())
                    {
                        auto *t = registry.getComponent<ecs::Transform>(enemyEntity);
                        auto *hb = registry.getComponent<ecs::Hitbox>(enemyEntity);
                        auto *vel = registry.getComponent<ecs::Velocity>(enemyEntity);
                        if (!t || !hb)
                            continue;
                        float hitboxY = t->y + hb->offsetY;
                        if (ceilingBottomY.has_value() && (hitboxY - hb->radius < ceilingBottomY.value()))
                        {
                            t->y = ceilingBottomY.value() + hb->radius - hb->offsetY;
                            if (vel)
                                vel->y = std::max(0.0f, vel->y);
                        }
                        if (floorTopY.has_value() && (hitboxY + hb->radius > floorTopY.value()))
                        {
                            t->y = floorTopY.value() - hb->radius - hb->offsetY;
                            if (vel)
                                vel->y = std::min(0.0f, vel->y);
                        }
                    }
                }
                if (m_wasPlayerPresent && !hasPlayer)
                {
                    playPlayerDeathSound(registry);
                }
                m_wasPlayerPresent = hasPlayer;
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            std::array<ecs::Entity, 4> m_enemyDeathAudioEntities{};
            ecs::Entity m_playerDeathAudioEntity = ecs::INVALID_ENTITY;
            std::size_t m_nextEnemyDeathChannel = 0;
            bool m_wasPlayerPresent = false;
            bool &m_showDebug;

            bool checkCircularCollision(const ecs::Transform &transform1, const ecs::Hitbox &hitbox1,
                                        const ecs::Transform &transform2, const ecs::Hitbox &hitbox2)
            {
                // Prendre en compte les offsets des hitbox
                float x1 = transform1.x + hitbox1.offsetX;
                float y1 = transform1.y + hitbox1.offsetY;
                float x2 = transform2.x + hitbox2.offsetX;
                float y2 = transform2.y + hitbox2.offsetY;

                float dx = x1 - x2;
                float dy = y1 - y2;
                float distance = std::sqrt(dx * dx + dy * dy);
                float combinedRadius = hitbox1.radius + hitbox2.radius;

                return distance < combinedRadius;
            }

            void removeProjectile(ecs::Registry &registry, ecs::Entity entity)
            {
                if (registry.hasComponent<ecs::Projectile>(entity))
                    registry.removeComponent<ecs::Projectile>(entity);
                if (registry.hasComponent<ecs::Transform>(entity))
                    registry.removeComponent<ecs::Transform>(entity);
                if (registry.hasComponent<ecs::Velocity>(entity))
                    registry.removeComponent<ecs::Velocity>(entity);
                if (registry.hasComponent<ecs::Rect>(entity))
                    registry.removeComponent<ecs::Rect>(entity);
                if (registry.hasComponent<ecs::Texture>(entity))
                    registry.removeComponent<ecs::Texture>(entity);
                if (registry.hasComponent<ecs::Scale>(entity))
                    registry.removeComponent<ecs::Scale>(entity);
                if (registry.hasComponent<ecs::Animation>(entity))
                    registry.removeComponent<ecs::Animation>(entity);
                if (registry.hasComponent<ecs::Hitbox>(entity))
                    registry.removeComponent<ecs::Hitbox>(entity);
            }

            void removeEnemy(ecs::Registry &registry, ecs::Entity entity)
            {
                if (registry.hasComponent<ecs::Enemy>(entity))
                    registry.removeComponent<ecs::Enemy>(entity);
                if (registry.hasComponent<ecs::Transform>(entity))
                    registry.removeComponent<ecs::Transform>(entity);
                if (registry.hasComponent<ecs::Velocity>(entity))
                    registry.removeComponent<ecs::Velocity>(entity);
                if (registry.hasComponent<ecs::Rect>(entity))
                    registry.removeComponent<ecs::Rect>(entity);
                if (registry.hasComponent<ecs::Texture>(entity))
                    registry.removeComponent<ecs::Texture>(entity);
                if (registry.hasComponent<ecs::Scale>(entity))
                    registry.removeComponent<ecs::Scale>(entity);
                if (registry.hasComponent<ecs::Animation>(entity))
                    registry.removeComponent<ecs::Animation>(entity);
                if (registry.hasComponent<ecs::Hitbox>(entity))
                    registry.removeComponent<ecs::Hitbox>(entity);
                if (registry.hasComponent<ecs::Projectile>(entity))
                    registry.removeComponent<ecs::Projectile>(entity);
            }

            void createExplosion(ecs::Registry &registry, float x, float y)
            {
                registry.createEntity()
                    .with<ecs::Transform>("explosion_transform", x, y, 0.0f)
                    .with<ecs::Rect>("explosion_rect", 0.0f, 0.0f,
                                     static_cast<int>(GameConfig::Explosion::SPRITE_WIDTH),
                                     static_cast<int>(GameConfig::Explosion::SPRITE_HEIGHT))
                    .with<ecs::Scale>("explosion_scale", GameConfig::Explosion::SCALE, GameConfig::Explosion::SCALE)
                    .with<ecs::Texture>("explosion_texture", Path::Texture::TEXTURE_EXPLOSION)
                    .with<ecs::Explosion>("explosion", 0, GameConfig::Explosion::ANIMATION_FRAMES,
                                          GameConfig::Explosion::ANIMATION_DURATION, 0.0f,
                                          GameConfig::Explosion::SPRITE_WIDTH, GameConfig::Explosion::SPRITE_HEIGHT,
                                          GameConfig::Explosion::FRAMES_PER_ROW, GameConfig::Explosion::LIFETIME, 0.0f)
                    .build();
            }

            void ensureEnemyDeathChannel(ecs::Registry &registry, std::size_t channelIndex)
            {
                if (channelIndex >= m_enemyDeathAudioEntities.size())
                    return;

                ecs::Entity &entity = m_enemyDeathAudioEntities[channelIndex];
                if (entity != ecs::INVALID_ENTITY && registry.hasComponent<ecs::Audio>(entity))
                {
                    return;
                }

                entity = registry.createEntity()
                             .with<ecs::Audio>("enemy_death_" + std::to_string(channelIndex),
                                               Path::Audio::AUDIO_DEATH_ENEMIES, 1.5F, false, false)
                             .build();
            }

            void playEnemyDeathSound(ecs::Registry &registry)
            {
                ensureEnemyDeathChannel(registry, m_nextEnemyDeathChannel);
                ecs::Entity entity = m_enemyDeathAudioEntities[m_nextEnemyDeathChannel];
                m_nextEnemyDeathChannel = (m_nextEnemyDeathChannel + 1) % m_enemyDeathAudioEntities.size();

                if (entity == ecs::INVALID_ENTITY)
                    return;

                if (auto *audio = registry.getComponent<ecs::Audio>(entity))
                {
                    audio->play = true;
                }
            }

            void ensurePlayerDeathAudio(ecs::Registry &registry)
            {
                if (m_playerDeathAudioEntity != ecs::INVALID_ENTITY &&
                    registry.hasComponent<ecs::Audio>(m_playerDeathAudioEntity))
                {
                    return;
                }

                m_playerDeathAudioEntity =
                    registry.createEntity()
                        .with<ecs::Audio>("player_death", Path::Audio::AUDIO_DEATH_ALLIES, 1.5F, false, false)
                        .build();
            }

            void playPlayerDeathSound(ecs::Registry &registry)
            {
                ensurePlayerDeathAudio(registry);
                if (m_playerDeathAudioEntity == ecs::INVALID_ENTITY)
                    return;

                if (auto *audio = registry.getComponent<ecs::Audio>(m_playerDeathAudioEntity))
                {
                    audio->play = true;
                }
            }
    }; // class CollisionSystem

} // namespace gme
