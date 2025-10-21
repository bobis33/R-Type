///
/// @file HUD.hpp
/// @brief HUD System for managing score display and UI elements
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Client/Common.hpp"

namespace cli
{

    class HUDSystem final : public eng::ASystem
    {
        public:
            explicit HUDSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~HUDSystem() override = default;

            HUDSystem(const HUDSystem &) = delete;
            HUDSystem &operator=(const HUDSystem &) = delete;
            HUDSystem(HUDSystem &&) = delete;
            HUDSystem &operator=(HUDSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override;
            void createScoreHUD(ecs::Registry &registry, float x, float y);
            void updateScore(ecs::Registry &registry, int newScore);

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            ecs::Entity m_scoreBgEntity;
            std::vector<ecs::Entity> m_scoreDigitEntities;
    }; // class HUDSystem

} // namespace cli
