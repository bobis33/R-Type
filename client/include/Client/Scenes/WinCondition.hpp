///
/// @file WinCondition.hpp
/// @brief This file contains the win condition scene
/// @namespace cli
///

#pragma once

#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"
#include "Utils/Clock.hpp"

namespace cli
{
    ///
    /// @class WinCondition
    /// @brief Win condition scene with beautiful animations
    /// @namespace cli
    ///
    class WinCondition final : public eng::AScene
    {
        public:
            WinCondition(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                         const std::shared_ptr<eng::IAudio> &audio);
            ~WinCondition() override = default;

            WinCondition(const WinCondition &other) = delete;
            WinCondition &operator=(const WinCondition &other) = delete;
            WinCondition(WinCondition &&other) = delete;
            WinCondition &operator=(WinCondition &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void()> onLeave;

        private:
            struct Particle
            {
                    float x;
                    float y;
                    float vx;
                    float vy;
                    float lifetime;
                    float maxLifetime;
                    ecs::Entity entity;
            };

            std::shared_ptr<eng::IRenderer> m_renderer;
            std::shared_ptr<eng::IAudio> m_audio;
            float m_elapsedTime = 0.F;
            float m_titlePulseTime = 0.F;
            float m_particleSpawnTimer = 0.F;

            ecs::Entity m_titleEntity = 0;
            ecs::Entity m_subtitleEntity = 0;
            ecs::Entity m_instructionEntity = 0;
            ecs::Entity m_soundEntity = 0;
            ecs::Entity m_iconEntity = 0;

            std::vector<Particle> m_particles;

            utl::Clock m_clock;
    }; // class WinCondition
} // namespace cli
