///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class ExplosionSystem final : public eng::ISystem
    {
        public:
            explicit ExplosionSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~ExplosionSystem() override = default;

            ExplosionSystem(const ExplosionSystem &) = delete;
            ExplosionSystem &operator=(const ExplosionSystem &) = delete;
            ExplosionSystem(ExplosionSystem &&) = delete;
            ExplosionSystem &operator=(ExplosionSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                std::vector<ecs::Entity> explosionsToRemove;

                for (auto &[entity, explosion] : registry.getAll<ecs::Explosion>())
                {
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    auto *texture = registry.getComponent<ecs::Texture>(entity);
                    auto *scale = registry.getComponent<ecs::Scale>(entity);

                    if (!transform || !rect || !texture || !scale)
                        continue;

                    explosion.current_time += dt;
                    if (explosion.current_time >= explosion.frame_duration)
                    {
                        explosion.current_time = 0.0f;
                        explosion.current_frame = (explosion.current_frame + 1) % explosion.total_frames;

                        int frame_x = (explosion.current_frame % explosion.frames_per_row) *
                                      static_cast<int>(explosion.frame_width);
                        int frame_y = (explosion.current_frame / explosion.frames_per_row) *
                                      static_cast<int>(explosion.frame_height);

                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }

                    m_renderer->createSprite(texture->id + std::to_string(entity), texture->path, transform->x,
                                            transform->y, scale->x, scale->y, static_cast<int>(rect->pos_x),
                                            static_cast<int>(rect->pos_y), static_cast<int>(rect->size_x),
                                            static_cast<int>(rect->size_y));
                    m_renderer->drawSprite(texture->id + std::to_string(entity));

                    explosion.current_lifetime += dt;
                    if (explosion.current_lifetime >= explosion.lifetime)
                    {
                        explosionsToRemove.push_back(entity);
                    }
                }

                for (ecs::Entity entity : explosionsToRemove)
                {
                    removeExplosion(registry, entity);
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;

            void removeExplosion(ecs::Registry &registry, const ecs::Entity entity)
            {
                if (registry.hasComponent<ecs::Explosion>(entity))
                    registry.removeComponent<ecs::Explosion>(entity);
                if (registry.hasComponent<ecs::Transform>(entity))
                    registry.removeComponent<ecs::Transform>(entity);
                if (registry.hasComponent<ecs::Rect>(entity))
                    registry.removeComponent<ecs::Rect>(entity);
                if (registry.hasComponent<ecs::Texture>(entity))
                    registry.removeComponent<ecs::Texture>(entity);
                if (registry.hasComponent<ecs::Scale>(entity))
                    registry.removeComponent<ecs::Scale>(entity);
            }
    };

} // namespace cli