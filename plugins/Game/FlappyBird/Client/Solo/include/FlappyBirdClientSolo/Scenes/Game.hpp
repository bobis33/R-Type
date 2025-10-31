///
/// @file Game.hpp
/// @brief This file contains the solo Game scene
/// @namespace gme
///

#pragma once

#include <memory>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace gme
{

    ///
    /// @class Game
    /// @brief Game solo scene
    /// @namespace gme
    ///
    class Game final : public eng::AScene
    {
        public:
            Game(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                     const std::shared_ptr<eng::IAudio> &audio, bool &showDebug);
            ~Game() override = default;

            Game(const Game &other) = delete;
            Game &operator=(const Game &other) = delete;
            Game(Game &&other) = delete;
            Game &operator=(Game &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

        private:
            static ecs::Entity createPlayer(ecs::Registry &registry);
            std::pair<ecs::Entity, ecs::Entity> createPipePair(ecs::Registry &registry, float x, float gapY) const;
            void resetGame();


            const std::shared_ptr<eng::IRenderer> &m_renderer;
            const std::shared_ptr<eng::IAudio> &m_audio;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_playerEntity;
            ecs::Entity m_looseText;
            ecs::Entity m_flapSound;
            ecs::Entity m_looseSound;
            std::vector<std::pair<ecs::Entity, ecs::Entity>> m_pipes;
            bool &m_showDebug;
            bool m_gameOver = false;
            bool m_gameOverShown = false;
    }; // class Game
} // namespace gme