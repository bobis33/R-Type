///
/// @file GameOver.hpp
/// @brief This file contains the game over scene
/// @namespace gme
///

#pragma once

#include <functional>

#include "Engine/Interfaces/IScene.hpp"

namespace gme
{
    ///
    /// @class GameOverScene
    /// @brief Game over scene displayed when all players die
    /// @namespace gme
    ///
    class GameOverScene final : public eng::AScene
    {
        public:
            GameOverScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~GameOverScene() override = default;

            GameOverScene(const GameOverScene &other) = delete;
            GameOverScene &operator=(const GameOverScene &other) = delete;
            GameOverScene(GameOverScene &&other) = delete;
            GameOverScene &operator=(GameOverScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            bool &playMusic() { return m_playMusic; }

            std::function<void()> onBackToMenu;

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            bool m_playMusic = false;
            float m_animationTime = 0.0f;
    }; // class GameOverScene
} // namespace gme
