///
/// @file ConfigMulti.hpp
/// @brief This file contains the multiplayer configuration scene
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    ///
    /// @class ConfigMulti
    /// @brief ConfigMulti scene
    /// @namespace cli
    ///
    class ConfigMulti final : public eng::AScene
    {
        public:
            ConfigMulti(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~ConfigMulti() override = default;

            ConfigMulti(const ConfigMulti &other) = delete;
            ConfigMulti &operator=(const ConfigMulti &other) = delete;
            ConfigMulti(ConfigMulti &&other) = delete;
            ConfigMulti &operator=(ConfigMulti &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void(const std::string &option)> onOptionSelected;

        private:
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_fpsEntity;
            const std::vector<std::string> m_menuOptions = {"Create room", "Join room", "Go back to menu"};
            const std::shared_ptr<eng::IAudio> &m_audio;

            int m_selectedIndex = 0;
    }; // class ConfigMulti
} // namespace cli