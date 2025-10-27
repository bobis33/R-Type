///
/// @file PlayerDirection.hpp
/// @brief This file contains the player direction system definitions
/// @namespace gme
///

#pragma once

#include <cmath>
#include <numbers>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "RTypeShared/GameConfig.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace gme
{
    struct AppConfig;

    class PlayerDirectionSystem final : public ecs::ASystem
    {
        public:
            explicit PlayerDirectionSystem(const int skinIndex) : m_skinIndex(skinIndex) {}
            ~PlayerDirectionSystem() override = default;

            PlayerDirectionSystem(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem &operator=(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem(PlayerDirectionSystem &&) = delete;
            PlayerDirectionSystem &operator=(PlayerDirectionSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (const auto &entity : registry.getAll<ecs::Player>() | std::views::keys)
                {
                    const auto *velocity = registry.getComponent<ecs::Velocity>(entity);

                    if (auto *rect = registry.getComponent<ecs::Rect>(entity);
                        (velocity != nullptr) && (rect != nullptr))
                    {
                        int frame = 0;
                        float angle = std::atan2(velocity->y, velocity->x);
                        if (std::abs(velocity->x) < 0.1f && std::abs(velocity->y) < 0.1f)
                        {
                            frame = 0;
                        }
                        else
                        {
                            if (angle < 0)
                            {
                                angle += 2.0f * std::numbers::pi_v<float>;
                            }
                            if (angle >= 0 && angle < M_PI / 4)
                            {
                                frame = 0;
                            }
                            else if (angle >= M_PI / 4 && angle < 3 * M_PI / 4)
                            {
                                frame = 1;
                            }
                            else if (angle >= 3 * M_PI / 4 && angle < 5 * M_PI / 4)
                            {
                                frame = 2;
                            }
                            else if (angle >= 5 * M_PI / 4 && angle < 7 * M_PI / 4)
                            {
                                frame = 3;
                            }
                            else
                            {
                                frame = 4;
                            }
                        }
                        int frame_width = static_cast<int>(GameConfig::Player::SPRITE_WIDTH);
                        int frame_height = static_cast<int>(GameConfig::Player::SPRITE_HEIGHT);
                        int frames_per_row = GameConfig::Player::FRAMES_PER_ROW;
                        int frame_x = (frame % frames_per_row) * frame_width;
                        int frame_y = (frame / frames_per_row) * frame_height;

                        const int skin_offset = m_skinIndex * static_cast<int>(GameConfig::Player::SPRITE_HEIGHT);
                        frame_y += skin_offset;

                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                        rect->size_x = frame_width;
                        rect->size_y = frame_height;
                    }
                }
            }

        private:
            int m_skinIndex;
    }; // class PlayerDirectionSystem
} // namespace gme