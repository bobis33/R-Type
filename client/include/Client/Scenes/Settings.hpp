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

            std::function<void()> onLeave;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            const std::shared_ptr<eng::IAudio> &m_audio;
    }; // class Settings
} // namespace cli