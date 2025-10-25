///
/// @file ConfigSolo.hpp
/// @brief This file contains the solo configuration scene
/// @namespace gme
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace gme
{
    ///
    /// @class ConfigSolo
    /// @brief ConfigSolo scene
    /// @namespace gme
    ///
    class ConfigSolo final : public eng::AScene
    {
        public:
            ConfigSolo(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                       const std::shared_ptr<eng::IAudio> &audio);
            ~ConfigSolo() override = default;

            ConfigSolo(const ConfigSolo &other) = delete;
            ConfigSolo &operator=(const ConfigSolo &other) = delete;
            ConfigSolo(ConfigSolo &&other) = delete;
            ConfigSolo &operator=(ConfigSolo &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void(const std::string &option)> onOptionSelected;

        private:
            void playInputSound();

            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_fpsEntity;
            ecs::Entity m_titleEntity;
            const std::vector<std::string> m_menuOptions = {"Level easy", "Level medium", "Go back to menu"};
            const std::shared_ptr<eng::IAudio> &m_audio;

            int m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            float m_titlePulseTime = 0.0f;

            // Audio pour navigation
            ecs::Entity m_selectionSoundEntity;
            std::string m_selectionSoundName;
    }; // class ConfigSolo
} // namespace gme