///
/// @file Beam.hpp
/// @brief This file contains the beam system definitions
/// @namespace ecs
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

namespace ecs
{

    class BeamSystem final : public ASystem
    {
        public:
            explicit BeamSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~BeamSystem() override = default;

            BeamSystem(const BeamSystem &) = delete;
            BeamSystem &operator=(const BeamSystem &) = delete;
            BeamSystem(BeamSystem &&) = delete;
            BeamSystem &operator=(BeamSystem &&) = delete;

            void update(Registry &registry, float /* dt */) override
            {
                for (auto &[entity, beamCharge] : registry.getAll<BeamCharge>())
                {
                    if (const auto *player = registry.getComponent<Player>(entity); player == nullptr)
                    {
                        continue;
                    }

                    const auto *transform = registry.getComponent<Transform>(entity);
                    if (transform == nullptr)
                    {
                        continue;
                    }

                    const float barX = transform->x + (utl::GameConfig::Player::SPRITE_WIDTH / 2.0f) -
                                       (utl::GameConfig::Beam::BAR_WIDTH / 2.0f);
                    const float barY = transform->y - utl::GameConfig::Beam::BAR_HEIGHT - 10.0f;

                    for (int x = 0; x < static_cast<int>(utl::GameConfig::Beam::BAR_WIDTH); x += 3)
                    {
                        for (int y = 0; y < static_cast<int>(utl::GameConfig::Beam::BAR_HEIGHT); y += 3)
                        {
                            m_renderer->drawPoint(barX + x, barY + y, {.r = 30, .g = 30, .b = 30, .a = 200});
                        }
                    }

                    const float chargeRatio = beamCharge.current_charge / beamCharge.max_charge;
                    const float chargeWidth = utl::GameConfig::Beam::BAR_WIDTH * chargeRatio;

                    const float thresholdX = barX + (utl::GameConfig::Beam::BAR_WIDTH * 0.5f);
                    for (int y = 0; y < static_cast<int>(utl::GameConfig::Beam::BAR_HEIGHT); y += 2)
                    {
                        m_renderer->drawPoint(thresholdX, barY + y, {.r = 255, .g = 255, .b = 255, .a = 150});
                    }

                    if (chargeWidth > 0)
                    {
                        for (int x = 0; x < static_cast<int>(chargeWidth); x += 3)
                        {
                            for (int y = 0; y < static_cast<int>(utl::GameConfig::Beam::BAR_HEIGHT); y += 3)
                            {
                                eng::Color chargeColor{};
                                if (chargeRatio < 0.5f)
                                {
                                    chargeColor = {.r = 255, .g = 100, .b = 0, .a = 255};
                                }
                                else if (chargeRatio < 0.8f)
                                {
                                    chargeColor = {.r = 255, .g = 200, .b = 0, .a = 255};
                                }
                                else
                                {
                                    chargeColor = {.r = 0, .g = 255, .b = 0, .a = 255};
                                }

                                m_renderer->drawPoint(barX + x, barY + y, chargeColor);
                            }
                        }
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class BeamSystem
} // namespace ecs