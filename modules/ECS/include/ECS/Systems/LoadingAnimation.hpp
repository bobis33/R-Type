///
/// @file LoadingAnimation.hpp
/// @brief This file contains the loading animation system definition
/// @namespace ecs
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace ecs
{

    ///
    /// @class LoadingAnimationSystem
    /// @brief Class for loading animation system
    /// @namespace ecs
    ///
    class LoadingAnimationSystem final : public ASystem
    {
        public:
            explicit LoadingAnimationSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~LoadingAnimationSystem() override = default;

            LoadingAnimationSystem(const LoadingAnimationSystem &) = delete;
            LoadingAnimationSystem &operator=(const LoadingAnimationSystem &) = delete;
            LoadingAnimationSystem(LoadingAnimationSystem &&) = delete;
            LoadingAnimationSystem &operator=(const LoadingAnimationSystem &&) = delete;

            void update(Registry &registry, float dt) override
            {
                std::string spriteName;
                spriteName.reserve(64);

                for (auto &[entity, animation] : registry.getAll<LoadingAnimation>())
                {
                    const auto *transform = registry.getComponent<Transform>(entity);
                    auto *rect = registry.getComponent<Rect>(entity);
                    const auto *texture = registry.getComponent<Texture>(entity);

                    if (!transform || !rect || !texture)
                    {
                        continue;
                    }

                    animation.current_time += dt;
                    if (animation.current_time >= animation.frame_duration)
                    {
                        animation.current_time = 0.0f;
                        animation.current_frame = (animation.current_frame + 1) % animation.total_frames;

                        const int frame_x = (animation.current_frame % animation.frames_per_row) *
                                            static_cast<int>(animation.frame_width);
                        const int frame_y = (animation.current_frame / animation.frames_per_row) *
                                            static_cast<int>(animation.frame_height);

                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }

                    spriteName.clear();
                    spriteName = texture->id;
                    spriteName += std::to_string(entity);

                    m_renderer->setSpriteTexture(spriteName, texture->path);
                    m_renderer->setSpritePosition(spriteName, transform->x, transform->y);
                    m_renderer->setSpriteFrame(spriteName, static_cast<int>(rect->pos_x), static_cast<int>(rect->pos_y),
                                               rect->size_x, rect->size_y);
                    m_renderer->drawSprite(spriteName);
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class LoadingAnimationSystem
} // namespace ecs

