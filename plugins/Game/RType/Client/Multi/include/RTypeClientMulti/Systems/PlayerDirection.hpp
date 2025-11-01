#pragma once

#include <cmath>
#include <utility>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace gme
{
    class PlayerDirectionSystem final : public ecs::ASystem
    {
        public:
            PlayerDirectionSystem() = default;
            ~PlayerDirectionSystem() override = default;

            PlayerDirectionSystem(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem &operator=(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem(PlayerDirectionSystem &&) = delete;
            PlayerDirectionSystem &operator=(PlayerDirectionSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &pair : registry.getAll<ecs::Player>())
                {
                    const auto entity = pair.first;
                    const auto *velocity = registry.getComponent<ecs::Velocity>(entity);

                    if (auto *rect = registry.getComponent<ecs::Rect>(entity);
                        (velocity != nullptr) && (rect != nullptr))
                    {
                        int frame = 0;
                        float angle = std::atan2(velocity->y, velocity->x);

                        const bool isIdle = (std::abs(velocity->x) < 0.1f && std::abs(velocity->y) < 0.1f);
                        if (isIdle)
                        {
                            frame = 0;
                        }
                        else
                        {
                            if (angle < 0)
                            {
                                angle += 2.0f * static_cast<float>(M_PI);
                            }
                            if (angle >= 0 && angle < M_PI / 4)
                            {
                                frame = 0; // droite
                            }
                            else if (angle >= M_PI / 4 && angle < 3 * M_PI / 4)
                            {
                                frame = 1; // bas
                            }
                            else if (angle >= 3 * M_PI / 4 && angle < 5 * M_PI / 4)
                            {
                                frame = 2; // gauche
                            }
                            else if (angle >= 5 * M_PI / 4 && angle < 7 * M_PI / 4)
                            {
                                frame = 3; // haut
                            }
                            else
                            {
                                frame = 4; // wrap
                            }
                        }

                        const int frame_x = (frame % utl::GameConfig::Player::FRAMES_PER_ROW) *
                                            static_cast<int>(utl::GameConfig::Player::SPRITE_WIDTH);
                        int frame_y = (frame / utl::GameConfig::Player::FRAMES_PER_ROW) *
                                      static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT);

                        const int current_row = static_cast<int>(
                            rect->pos_y / static_cast<float>(static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT)));
                        const int skin_offset = current_row * static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT);
                        frame_y = skin_offset + frame_y;

                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                        rect->size_x = static_cast<int>(utl::GameConfig::Player::SPRITE_WIDTH);
                        rect->size_y = static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT);
                    }
                }
            }
    };
} // namespace gme
