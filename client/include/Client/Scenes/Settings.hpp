///
/// @file Settings.hpp
/// @brief This file contains the settings scene
/// @namespace cli
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"

namespace cli
{
    struct AppConfig;

    ///
    /// @class Settings
    /// @brief Settings scene
    /// @namespace cli
    ///
    class Settings final : public eng::AScene
    {
        public:
            Settings(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer, AppConfig &config);
            ~Settings() override = default;

            Settings(const Settings &other) = delete;
            Settings &operator=(const Settings &other) = delete;
            Settings(Settings &&other) = delete;
            Settings &operator=(Settings &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            void updateSettingsDisplay();
            void loadFromConfig();
            bool &playMusic() { return m_playMusic; }

            std::function<void()> onLeave;

        private:
            static void applyVideoQuality();
            static void applySkinChange();

            std::unordered_map<eng::Key, bool> m_keysPressed;
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            AppConfig &m_appConfig;
            size_t m_selectedIndex = 0;
            const std::vector<std::string> m_settingsOptions = {"Audio Volume", "FPS", "Controls", "Skin",
                                                                "Back to Menu"};
            float m_audioVolume = 50.0F;
            int m_videoQuality = 1;
            int m_controlScheme = 2;
            int m_skinIndex = 0;
            ecs::Entity m_volumeValueEntity;
            ecs::Entity m_qualityValueEntity;
            ecs::Entity m_controlValueEntity;
            ecs::Entity m_skinSpriteEntity;
            ecs::Entity m_titleEntity;
            float m_animationTime = 0.0f;
            float m_titlePulseTime = 0.0f;
            bool m_playMusic = false;
    }; // class Settings
} // namespace cli