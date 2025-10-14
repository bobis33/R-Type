///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class LoadingAnimationSystem final : public eng::ASystem
    {
        public:
            explicit LoadingAnimationSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~LoadingAnimationSystem() override = default;

            LoadingAnimationSystem(const LoadingAnimationSystem &) = delete;
            LoadingAnimationSystem &operator=(const LoadingAnimationSystem &) = delete;
            LoadingAnimationSystem(LoadingAnimationSystem &&) = delete;
            LoadingAnimationSystem &operator=(LoadingAnimationSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                for (auto &[entity, animation] : registry.getAll<ecs::LoadingAnimation>())
                {
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    const auto *texture = registry.getComponent<ecs::Texture>(entity);

                    if (!transform || !rect || !texture)
                        continue;

                    // Mettre à jour l'animation
                    animation.current_time += dt;
                    if (animation.current_time >= animation.frame_duration)
                    {
                        animation.current_time = 0.0f;
                        animation.current_frame = (animation.current_frame + 1) % animation.total_frames;

                        // Mettre à jour le rectangle de texture
                        int frame_x = (animation.current_frame % animation.frames_per_row) *
                                      static_cast<int>(animation.frame_width);
                        int frame_y = (animation.current_frame / animation.frames_per_row) *
                                      static_cast<int>(animation.frame_height);

                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }

                    // Dessiner l'animation
                    m_renderer->setSpriteTexture(texture->id + std::to_string(entity), texture->path);
                    m_renderer->setSpritePosition(texture->id + std::to_string(entity), transform->x, transform->y);
                    m_renderer->setSpriteFrame(texture->id + std::to_string(entity), static_cast<int>(rect->pos_x),
                                              static_cast<int>(rect->pos_y), rect->size_x, rect->size_y);
                    m_renderer->drawSprite(texture->id + std::to_string(entity));
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class LoadingAnimationSystem

} // namespace cli