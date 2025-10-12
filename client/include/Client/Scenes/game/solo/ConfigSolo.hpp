///
/// @file ConfigSolo.hpp
/// @brief This file contains the solo configuration scene
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    ///
    /// @class ConfigSolo
    /// @brief ConfigSolo scene
    /// @namespace cli
    ///
    class ConfigSolo final : public eng::AScene
    {
        public:
            ConfigSolo(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~ConfigSolo() override = default;

            ConfigSolo(const ConfigSolo &other) = delete;
            ConfigSolo &operator=(const ConfigSolo &other) = delete;
            ConfigSolo(ConfigSolo &&other) = delete;
            ConfigSolo &operator=(ConfigSolo &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void(const std::string &option)> onOptionSelected;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_fpsEntity;
            const std::vector<std::string> m_menuOptions = {"Level easy", "Level medium", "Go back to menu"};
            const std::shared_ptr<eng::IAudio> &m_audio;

            int m_selectedIndex = 0;
    }; // class ConfigSolo
} // namespace cli