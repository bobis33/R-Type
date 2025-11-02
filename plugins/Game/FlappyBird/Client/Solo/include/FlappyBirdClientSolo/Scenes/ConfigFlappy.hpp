///
/// @file ConfigFlappy.hpp
/// @brief This file contains the FlappyBird configuration scene
/// @namespace gme
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"

namespace gme
{
    ///
    /// @class ConfigFlappy
    /// @brief ConfigFlappy scene
    /// @namespace gme
    ///
    class ConfigFlappy final : public eng::AScene
    {
        public:
            ConfigFlappy(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~ConfigFlappy() override = default;

            ConfigFlappy(const ConfigFlappy &other) = delete;
            ConfigFlappy &operator=(const ConfigFlappy &other) = delete;
            ConfigFlappy(ConfigFlappy &&other) = delete;
            ConfigFlappy &operator=(const ConfigFlappy &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            bool &playMusic() { return m_playMusic; }

            std::function<void(const std::string &option, const std::string &playerName, int skinIndex)> onOptionSelected;

        private:
            char keyToChar(eng::Key key) const;
            std::string &getCurrentEditField();
            void updateValueDisplay();

            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_fpsEntity;
            ecs::Entity m_titleEntity;
            ecs::Entity m_playerNameValueEntity;
            ecs::Entity m_skinValueEntity;
            ecs::Entity m_skinSpriteEntity;
            const std::vector<std::string> m_menuOptions = {"Player Name", "Skin", "Start Game", "Go back to menu"};

            int m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            float m_titlePulseTime = 0.0f;
            bool m_playMusic = false;
            std::string m_playerName = "Player";
            int m_skinIndex = 0;
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class ConfigFlappy
} // namespace gme

