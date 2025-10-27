///
/// @file ConfigMulti.hpp
/// @brief This file contains the multiplayer configuration scene
/// @namespace gme
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"

namespace gme
{
    ///
    /// @class ConfigMulti
    /// @brief ConfigMulti scene
    /// @namespace gme
    ///
    class ConfigMulti final : public eng::AScene
    {
        public:
            ConfigMulti(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~ConfigMulti() override = default;

            ConfigMulti(const ConfigMulti &other) = delete;
            ConfigMulti &operator=(const ConfigMulti &other) = delete;
            ConfigMulti(ConfigMulti &&other) = delete;
            ConfigMulti &operator=(ConfigMulti &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            bool &playMusic() { return m_playMusic; }

            std::function<void(const std::string &option)> onOptionSelected;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_titleEntity;
            const std::vector<std::string> m_menuOptions = {"Create room", "Join room", "Go back to menu"};
            bool m_playMusic = false;
            int m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            float m_titlePulseTime = 0.0f;
    }; // class ConfigMulti
} // namespace gme