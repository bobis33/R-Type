///
/// @file GameSolo.hpp
/// @brief This file contains the solo Game scene
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    ///
    /// @class Game
    /// @brief GameSolo scene
    /// @namespace cli
    ///
    class GameSolo final : public eng::AScene
    {
        public:
            GameSolo(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~GameSolo() override = default;

            GameSolo(const GameSolo &other) = delete;
            GameSolo &operator=(const GameSolo &other) = delete;
            GameSolo(GameSolo &&other) = delete;
            GameSolo &operator=(GameSolo &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;

            ecs::Entity m_playerEntity;
            ecs::Entity m_fpsEntity;
            ecs::Entity m_enemyCounterEntity;
            ecs::Entity m_asteroidCounterEntity;
            const std::shared_ptr<eng::IAudio> &m_audio;
    }; // class GameSolo
} // namespace cli