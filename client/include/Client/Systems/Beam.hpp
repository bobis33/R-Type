///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "Client/GameConfig.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class BeamSystem final : public eng::ASystem
    {
        public:
            explicit BeamSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~BeamSystem() override = default;

            BeamSystem(const BeamSystem &) = delete;
            BeamSystem &operator=(const BeamSystem &) = delete;
            BeamSystem(BeamSystem &&) = delete;
            BeamSystem &operator=(BeamSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                // Chercher seulement le joueur avec BeamCharge
                for (auto &[entity, beamCharge] : registry.getAll<ecs::BeamCharge>())
                {
                    const auto *player = registry.getComponent<ecs::Player>(entity);
                    if (!player)
                        continue; // Seulement pour le joueur

                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    if (!transform)
                        continue;

                    // Position de la barre au-dessus du joueur
                    float barX =
                        transform->x + GameConfig::Player::SPRITE_WIDTH / 2.0f - GameConfig::Beam::BAR_WIDTH / 2.0f;
                    float barY = transform->y - GameConfig::Beam::BAR_HEIGHT - 10.0f; // 10 pixels au-dessus

                    // Dessiner la barre de fond avec des points (plus petite et plus fine)
                    for (int x = 0; x < static_cast<int>(GameConfig::Beam::BAR_WIDTH); x += 3)
                    {
                        for (int y = 0; y < static_cast<int>(GameConfig::Beam::BAR_HEIGHT); y += 3)
                        {
                            m_renderer->drawPoint(barX + x, barY + y, {.r = 30, .g = 30, .b = 30, .a = 200});
                        }
                    }

                    // Dessiner la barre de chargement
                    float chargeRatio = beamCharge.current_charge / beamCharge.max_charge;
                    float chargeWidth = GameConfig::Beam::BAR_WIDTH * chargeRatio;

                    // Dessiner le seuil de 50% (ligne verticale)
                    float thresholdX = barX + GameConfig::Beam::BAR_WIDTH * 0.5f;
                    for (int y = 0; y < static_cast<int>(GameConfig::Beam::BAR_HEIGHT); y += 2)
                    {
                        m_renderer->drawPoint(
                            thresholdX, barY + y,
                            {.r = 255, .g = 255, .b = 255, .a = 150}); // Ligne blanche semi-transparente
                    }

                    if (chargeWidth > 0)
                    {
                        for (int x = 0; x < static_cast<int>(chargeWidth); x += 3)
                        {
                            for (int y = 0; y < static_cast<int>(GameConfig::Beam::BAR_HEIGHT); y += 3)
                            {
                                // Couleur qui change selon le niveau de charge
                                eng::Color chargeColor;
                                if (chargeRatio < 0.5f)
                                {
                                    chargeColor = {
                                        .r = 255, .g = 100, .b = 0, .a = 255}; // Orange (en dessous du seuil)
                                }
                                else if (chargeRatio < 0.8f)
                                {
                                    chargeColor = {.r = 255, .g = 200, .b = 0, .a = 255}; // Jaune
                                }
                                else
                                {
                                    chargeColor = {.r = 0, .g = 255, .b = 0, .a = 255}; // Vert
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

} // namespace cli