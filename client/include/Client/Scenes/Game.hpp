///
/// @file Game.hpp
/// @brief This file contains the Game scene
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include "Client/Systems/Weapon.hpp"
#include "Engine/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    ///
    /// @class Game
    /// @brief Game scene
    /// @namespace cli
    ///
    class Game final : public eng::AScene
    {
        public:
            Game(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~Game() override = default;

            Game(const Game &other) = delete;
            Game &operator=(const Game &other) = delete;
            Game(Game &&other) = delete;
            Game &operator=(Game &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;

            ecs::Entity m_playerEntity;
            ecs::Entity m_fpsEntity;
            ecs::Entity m_enemyCounterEntity;
            ecs::Entity m_asteroidCounterEntity;
            const std::shared_ptr<eng::IAudio> &m_audio;

            // WeaponSystem m_weaponSystem; TODO(bobis33): tofix
    }; // class Game
} // namespace cli