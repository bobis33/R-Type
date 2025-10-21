///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include <cmath>
#include <vector>

#include "Client/GameConfig.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class CollisionSystem final : public eng::ISystem
    {
        public:
            explicit CollisionSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~CollisionSystem() override = default;

            CollisionSystem(const CollisionSystem &) = delete;
            CollisionSystem &operator=(const CollisionSystem &) = delete;
            CollisionSystem(CollisionSystem &&) = delete;
            CollisionSystem &operator=(CollisionSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
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
                        if (ceilingBottomY.has_value() && (t->y - hb->radius < ceilingBottomY.value()))
                        {
                            t->y = ceilingBottomY.value() + hb->radius;
                            if (vel)
                                vel->y = std::max(0.0f, vel->y);
                        }
                        if (floorTopY.has_value() && (t->y + hb->radius > floorTopY.value()))
                        {
                            t->y = floorTopY.value() - hb->radius;
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
                        if (ceilingBottomY.has_value() && (t->y - hb->radius < ceilingBottomY.value()))
                        {
                            t->y = ceilingBottomY.value() + hb->radius;
                            if (vel)
                                vel->y = std::max(0.0f, vel->y);
                        }
                        if (floorTopY.has_value() && (t->y + hb->radius > floorTopY.value()))
                        {
                            t->y = floorTopY.value() - hb->radius;
                            if (vel)
                                vel->y = std::min(0.0f, vel->y);
                        }
                    }
                }
            }
        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;

            bool checkCircularCollision(const ecs::Transform &transform1, const ecs::Hitbox &hitbox1,
                                        const ecs::Transform &transform2, const ecs::Hitbox &hitbox2)
            {
                float dx = transform1.x - transform2.x;
                float dy = transform1.y - transform2.y;
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
    }; // class CollisionSystem

} // namespace cli