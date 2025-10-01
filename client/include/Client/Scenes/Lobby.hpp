///
/// @file LobbyScene.hpp
/// @brief This file contains the lobby scene
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include "Engine/IScene.hpp"
#include "Interfaces/IAudio.hpp"
#include "Client/WeaponSystem.hpp"

namespace cli
{
    ///
    /// @class Lobby
    /// @brief Lobby scene
    /// @namespace cli
    ///
    class Lobby final : public eng::AScene
    {
        public:
            Lobby(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~Lobby() override = default;

            Lobby(const Lobby &other) = delete;
            Lobby &operator=(const Lobby &other) = delete;
            Lobby(Lobby &&other) = delete;
            Lobby &operator=(Lobby &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;

            ecs::Entity m_playerEntity;
            ecs::Entity m_fpsEntity;
            ecs::Entity m_enemyCounterEntity;
            ecs::Entity m_asteroidCounterEntity;
            
            // Système d'armes
            WeaponSystem m_weaponSystem;
    }; // class Lobby
} // namespace cli