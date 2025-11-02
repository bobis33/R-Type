///
/// @file Animation.hpp
/// @brief Animation system for multiplayer
/// @namespace gme
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{

    class AnimationSystem final : public ecs::ASystem
    {
        public:
            explicit AnimationSystem(const std::shared_ptr<eng::IRenderer> & /* renderer */) {}
            ~AnimationSystem() override = default;

            AnimationSystem(const AnimationSystem &) = delete;
            AnimationSystem &operator=(const AnimationSystem &) = delete;
            AnimationSystem(AnimationSystem &&) = delete;
            AnimationSystem &operator=(const AnimationSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                for (auto &[entity, animation] : registry.getAll<ecs::Animation>())
                {
                    animation.current_time += dt;

                    if (animation.current_time >= animation.frame_duration)
                    {
                        animation.current_time = 0.0f;
                        animation.current_frame = (animation.current_frame + 1) % animation.total_frames;
                    }

                    if (auto *rect = registry.getComponent<ecs::Rect>(entity))
                    {
                        const int frame_x =
                            (animation.current_frame % animation.frames_per_row) * animation.frame_width;
                        if (int frame_y = (animation.current_frame / animation.frames_per_row) * animation.frame_height;
                            rect->pos_x != static_cast<float>(frame_x) || rect->pos_y != static_cast<float>(frame_y))
                        {
                            rect->pos_x = static_cast<float>(frame_x);
                            rect->pos_y = static_cast<float>(frame_y);
                            rect->size_x = animation.frame_width;
                            rect->size_y = animation.frame_height;
                        }
                    }
                }
            }

    }; // class AnimationSystem
} // namespace gme
