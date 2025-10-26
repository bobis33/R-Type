///
/// @file Starfield.hpp
/// @brief Starfield System for managing background stars and visual effects
/// @namespace gme
///

#pragma once

#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{

    class StarfieldSystem final : public ecs::ASystem
    {
        public:
            explicit StarfieldSystem(const std::shared_ptr<eng::IRenderer> &renderer, ecs::Registry &registry) : m_renderer(renderer) { createStarfield(registry, m_renderer->getWindowSize());}
            ~StarfieldSystem() override = default;

            StarfieldSystem(const StarfieldSystem &) = delete;
            StarfieldSystem &operator=(const StarfieldSystem &) = delete;
            StarfieldSystem(StarfieldSystem &&) = delete;
            StarfieldSystem &operator=(StarfieldSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override;

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;

            static void createStarfield(ecs::Registry &registry, const eng::WindowSize &windowSize);
            static void createStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight,
                             const eng::Color &color, float velocity, const std::string &id);
            static void createShootingStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
            static void createPlanets(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
            static void createNebulae(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
            static void createComets(ecs::Registry &registry, int count, int screenWidth, int screenHeight);
    }; // class StarfieldSystem

} // namespace gme
