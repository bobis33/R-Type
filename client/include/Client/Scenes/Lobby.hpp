///
/// @file Lobby.hpp
/// @brief This file contains the lobby scene
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include "Client/Systems/Weapon.hpp"
#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

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

            std::function<void(const std::string &option)> onOptionSelected;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_fpsEntity;
            const std::vector<std::string> m_menuOptions = {"Solo", "Multi", "Settings"};
            const std::shared_ptr<eng::IAudio> &m_audio;

            int m_selectedIndex = 0;
    }; // class Lobby
} // namespace cli