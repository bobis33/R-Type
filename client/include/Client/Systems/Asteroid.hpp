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

    class AsteroidSystem final : public eng::ISystem
    {
        public:
            explicit AsteroidSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~AsteroidSystem() override = default;

            AsteroidSystem(const AsteroidSystem &) = delete;
            AsteroidSystem &operator=(const AsteroidSystem &) = delete;
            AsteroidSystem(AsteroidSystem &&) = delete;
            AsteroidSystem &operator=(AsteroidSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                std::vector<ecs::Entity> asteroidsToRemove;

                for (auto &[entity, asteroid] : registry.getAll<ecs::Asteroid>())
                {
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *velocity = registry.getComponent<ecs::Velocity>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    auto *texture = registry.getComponent<ecs::Texture>(entity);
                    auto *scale = registry.getComponent<ecs::Scale>(entity);
                    auto *animation = registry.getComponent<ecs::Animation>(entity);

                    if (!transform || !velocity || !rect || !texture || !scale || !animation)
                        continue;

                    animation->current_time += dt;
                    if (animation->current_time >= animation->frame_duration)
                    {
                        animation->current_time = 0.0f;
                        animation->current_frame = (animation->current_frame + 1) % animation->total_frames;

                        const int frame_x =
                            animation->current_frame * static_cast<int>(GameConfig::Asteroid::Small::SPRITE_WIDTH);
                        const int frame_y = 0;
                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }

                    transform->x += velocity->x * dt;
                    transform->y += velocity->y * dt;
                    transform->rotation += asteroid.rotation_speed * dt;

                    m_renderer.createSprite(texture->id + std::to_string(entity), texture->path,
                                            std::round(transform->x), std::round(transform->y), scale->x, scale->y,
                                            static_cast<int>(rect->pos_x), static_cast<int>(rect->pos_y),
                                            static_cast<int>(rect->size_x), static_cast<int>(rect->size_y));
                    m_renderer.drawSprite(texture->id + std::to_string(entity));

                    if (transform->x < GameConfig::Screen::REMOVE_X ||
                        transform->y < GameConfig::Screen::REMOVE_MIN_Y ||
                        transform->y > GameConfig::Screen::REMOVE_MAX_Y)
                    {
                        asteroidsToRemove.push_back(entity);
                    }
                }

                for (ecs::Entity entity : asteroidsToRemove)
                {
                    if (registry.hasComponent<ecs::Asteroid>(entity))
                        registry.removeComponent<ecs::Asteroid>(entity);
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
                    if (registry.hasComponent<ecs::Hitbox>(entity))
                        registry.removeComponent<ecs::Hitbox>(entity);
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class AsteroidSystem

} // namespace cli