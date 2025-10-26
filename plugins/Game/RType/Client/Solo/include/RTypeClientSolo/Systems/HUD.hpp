///
/// @file HUD.hpp
/// @brief HUD System for managing score display and UI elements
/// @namespace gme
///

#pragma once

#include "ECS/Interfaces/ISystems.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Common.hpp"

namespace gme
{

    class HUDSystem final : public ecs::ASystem
    {
        public:
            explicit HUDSystem(const std::shared_ptr<eng::IRenderer> &renderer, ecs::Registry &registry) : m_renderer(renderer), m_registry(registry) { createScoreHUD(m_registry, 10.0F, 10.0F); }
            ~HUDSystem() override = default;

            HUDSystem(const HUDSystem &) = delete;
            HUDSystem &operator=(const HUDSystem &) = delete;
            HUDSystem(HUDSystem &&) = delete;
            HUDSystem &operator=(HUDSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override;

        private:
            void createScoreHUD(ecs::Registry &registry, float x, float y);
            void updateScore(ecs::Registry &registry, int newScore) const;

            const std::shared_ptr<eng::IRenderer> &m_renderer;
            ecs::Registry &m_registry;
            ecs::Entity m_scoreBgEntity{};
            std::vector<ecs::Entity> m_scoreDigitEntities;
    }; // class HUDSystem

} // namespace gme
