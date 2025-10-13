///
/// @file Settings.hpp
/// @brief This file contains the settings scene
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    ///
    /// @class Settings
    /// @brief Settings scene
    /// @namespace cli
    ///
    class Settings final : public eng::AScene
    {
        public:
            Settings(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~Settings() override = default;

            Settings(const Settings &other) = delete;
            Settings &operator=(const Settings &other) = delete;
            Settings(Settings &&other) = delete;
            Settings &operator=(Settings &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            void updateSettingsDisplay();

            std::function<void()> onLeave;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            const std::shared_ptr<eng::IAudio> &m_audio;
            
            size_t m_selectedIndex = 0;
            const std::vector<std::string> m_settingsOptions = {"Audio Volume", "Video Quality", "Controls", "Skin", "Back to Menu"};
            
            int m_audioVolume = 50;
            size_t m_videoQuality = 1;
            size_t m_controlScheme = 0;
            size_t m_skinIndex = 0;
            
            ecs::Entity m_volumeValueEntity;
            ecs::Entity m_qualityValueEntity;
            ecs::Entity m_controlValueEntity;
            ecs::Entity m_skinSpriteEntity;
            ecs::Entity m_titleEntity;
            
            float m_animationTime = 0.0f;
            float m_titlePulseTime = 0.0f;
    }; // class Settings
} // namespace cli