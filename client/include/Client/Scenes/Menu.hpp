///
/// @file Menu.hpp
/// @brief This file contains the menu scene
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
    /// @class Menu
    /// @brief Menu scene
    /// @namespace cli
    ///
    class Menu final : public eng::AScene
    {
        public:
            Menu(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~Menu() override = default;

            Menu(const Menu &other) = delete;
            Menu &operator=(const Menu &other) = delete;
            Menu(Menu &&other) = delete;
            Menu &operator=(Menu &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void(const std::string &option)> onOptionSelected;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_fpsEntity;
            const std::vector<std::string> m_menuOptions = {"Solo", "Multi", "Settings"};
            const std::shared_ptr<eng::IAudio> &m_audio;

            int m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            float m_titlePulseTime = 0.0f;
            ecs::Entity m_titleEntity;

            const std::vector<std::string> m_contributors = {"Elliot", "Arthur", "Moana", "Aaron"};
            ecs::Entity m_contributorsEntity;
            float m_contributorsOffset = 0.0f;
    }; // class Menu
} // namespace cli