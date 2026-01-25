///
/// @file Intro.hpp
/// @brief This file contains the intro scene
/// @namespace cli
///

#pragma once

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"
#include "Utils/Clock.hpp"

namespace cli
{
    ///
    /// @class Intro
    /// @brief Intro scene
    /// @namespace cli
    ///
    class Intro final : public eng::AScene
    {
        public:
            Intro(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                  const std::shared_ptr<eng::IAudio> &audio);
            ~Intro() override = default;

            Intro(const Intro &other) = delete;
            Intro &operator=(const Intro &other) = delete;
            Intro(Intro &&other) = delete;
            Intro &operator=(Intro &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void()> onLeave;

        private:
            std::shared_ptr<eng::IRenderer> m_renderer;
            std::shared_ptr<eng::IAudio> m_audio;
            float m_elapsedTime = 0.F;

            ecs::Entity m_logoEntity = 0;
            utl::Clock m_clock;
    }; // class Intro
} // namespace cli
