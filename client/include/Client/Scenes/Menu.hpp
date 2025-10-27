///
/// @file Menu.hpp
/// @brief This file contains the menu scene
/// @namespace cli
///

#pragma once

#include <unordered_map>
#include <vector>

#include "Engine/Interfaces/IScene.hpp"

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
            Menu(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~Menu() override = default;

            Menu(const Menu &other) = delete;
            Menu &operator=(const Menu &other) = delete;
            Menu(Menu &&other) = delete;
            Menu &operator=(Menu &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            bool &playMusic() { return m_playMusic; }

            std::function<void(const std::string &option)> onOptionSelected;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            const std::vector<std::string> m_menuOptions = {"Solo", "Multi", "Settings"};

            int m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            float m_titlePulseTime = 0.0f;
            ecs::Entity m_titleEntity;

            const std::vector<std::string> m_contributors = {"Elliot", "Arthur", "Moana", "Aaron"};
            ecs::Entity m_contributorsEntity;
            float m_contributorsOffset = 0.0f;
            bool m_playMusic = false;
    }; // class Menu
} // namespace cli
