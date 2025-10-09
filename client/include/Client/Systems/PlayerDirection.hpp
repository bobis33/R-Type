///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include <cmath>

#include "Client/GameConfig.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cli
{

    class PlayerDirectionSystem final : public eng::ASystem
    {
        public:
            explicit PlayerDirectionSystem() = default;
            ~PlayerDirectionSystem() override = default;

            PlayerDirectionSystem(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem &operator=(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem(PlayerDirectionSystem &&) = delete;
            PlayerDirectionSystem &operator=(PlayerDirectionSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, player] : registry.getAll<ecs::Player>())
                {
                    const auto *velocity = registry.getComponent<ecs::Velocity>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);

                    if (velocity && rect)
                    {
                        int frame = 0;
                        float angle = std::atan2(velocity->y, velocity->x);
                        if (std::abs(velocity->x) < 0.1f && std::abs(velocity->y) < 0.1f)
                        {
                            return;
                        }
                        if (angle < 0)
                            angle += 2.0f * static_cast<float>(M_PI);
                        if (angle >= 0 && angle < M_PI / 4)
                            frame = 0; // Droite
                        else if (angle >= M_PI / 4 && angle < 3 * M_PI / 4)
                            frame = 1; // Haut
                        else if (angle >= 3 * M_PI / 4 && angle < 5 * M_PI / 4)
                            frame = 2; // Gauche
                        else if (angle >= 5 * M_PI / 4 && angle < 7 * M_PI / 4)
                            frame = 3; // Bas
                        else
                            frame = 4; // Droite (retour)
                        int frame_width = static_cast<int>(GameConfig::Player::SPRITE_WIDTH);
                        int frame_height = static_cast<int>(GameConfig::Player::SPRITE_HEIGHT);
                        int frames_per_row = GameConfig::Player::FRAMES_PER_ROW;
                        int frame_x = (frame % frames_per_row) * frame_width;
                        int frame_y = (frame / frames_per_row) * frame_height;

                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                        rect->size_x = frame_width;
                        rect->size_y = frame_height;
                    }
                }
            }
    }; // class PlayerDirectionSystem

} // namespace cli