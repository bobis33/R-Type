///
/// @file Explosion.hpp
/// @brief This file contains the explosion system definition
/// @namespace ecs
///

#pragma once

#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace ecs
{

    ///
    /// @class ExplosionSystem
    /// @brief Class for explosion system
    /// @namespace ecs
    ///
    class ExplosionSystem final : public ASystem
    {
        public:
            explicit ExplosionSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~ExplosionSystem() override = default;

            ExplosionSystem(const ExplosionSystem &) = delete;
            ExplosionSystem &operator=(const ExplosionSystem &) = delete;
            ExplosionSystem(ExplosionSystem &&) = delete;
            ExplosionSystem &operator=(const ExplosionSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(Registry &registry, float dt) override
            {
                std::vector<Entity> explosionsToRemove;

                for (auto &[entity, explosion] : registry.getAll<Explosion>())
                {
                    auto *transform = registry.getComponent<Transform>(entity);
                    auto *rect = registry.getComponent<Rect>(entity);
                    auto *texture = registry.getComponent<Texture>(entity);
                    auto *scale = registry.getComponent<Scale>(entity);

                    if ((transform == nullptr) || (rect == nullptr) || (texture == nullptr) || (scale == nullptr))
                    {
                        continue;
                    }

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
                                             static_cast<int>(rect->pos_y), rect->size_x, rect->size_y);
                    m_renderer->drawSprite(texture->id + std::to_string(entity));

                    explosion.current_lifetime += dt;
                    if (explosion.current_lifetime >= explosion.lifetime)
                    {
                        explosionsToRemove.push_back(entity);
                    }
                }

                for (const Entity entity : explosionsToRemove)
                {
                    removeExplosion(registry, entity);
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;

            static void removeExplosion(Registry &registry, const Entity entity)
            {
                if (registry.hasComponent<Explosion>(entity))
                    registry.removeComponent<Explosion>(entity);
                if (registry.hasComponent<Transform>(entity))
                    registry.removeComponent<Transform>(entity);
                if (registry.hasComponent<Rect>(entity))
                    registry.removeComponent<Rect>(entity);
                if (registry.hasComponent<Texture>(entity))
                    registry.removeComponent<Texture>(entity);
                if (registry.hasComponent<Scale>(entity))
                    registry.removeComponent<Scale>(entity);
            }
    }; // class ExplosionSystem
} // namespace ecs

