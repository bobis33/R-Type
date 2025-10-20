///
/// @file Starfield.hpp
/// @brief Starfield System for managing background stars and visual effects
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

    class StarfieldSystem final : public eng::ASystem
    {
        public:
            explicit StarfieldSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~StarfieldSystem() override = default;

            StarfieldSystem(const StarfieldSystem &) = delete;
            StarfieldSystem &operator=(const StarfieldSystem &) = delete;
            StarfieldSystem(StarfieldSystem &&) = delete;
            StarfieldSystem &operator=(StarfieldSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override;
            void createStarfield(ecs::Registry &registry, int screenWidth, int screenHeight);

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            
            void createStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight, 
                           const eng::Color &color, float velocity, const std::string &id);
            void createShootingStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
            void createPlanets(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
            void createNebulae(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
            void createComets(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
    }; // class StarfieldSystem

} // namespace cli
