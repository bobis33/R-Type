///
/// @file GameSolo.hpp
/// @brief This file contains the solo Game scene
/// @namespace eng
///

#pragma once

#include <memory>

#include "Client/Managers/StageManager.hpp"
#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    class HUDSystem;
    class StarfieldSystem;
    class PlayerController;
} // namespace cli

namespace cli
{
    struct AppConfig;
    ///
    /// @class Game
    /// @brief GameSolo scene
    /// @namespace cli
    ///
    class GameSolo final : public eng::AScene
    {
        public:
            GameSolo(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio,
                     const AppConfig &appConfig);
            ~GameSolo() override = default;

            GameSolo(const GameSolo &other) = delete;
            GameSolo &operator=(const GameSolo &other) = delete;
            GameSolo(GameSolo &&other) = delete;
            GameSolo &operator=(GameSolo &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            void updatePlayerSkin();

        private:
            ecs::Entity m_playerEntity;
            const std::shared_ptr<eng::IAudio> &m_audio;
            const AppConfig &m_appConfig;
            std::unique_ptr<HUDSystem> m_hudSystem;
            std::unique_ptr<StarfieldSystem> m_starfieldSystem;
            std::unique_ptr<PlayerController> m_playerController;
            std::unique_ptr<StageManager> m_stageManager;
            bool isUpPressed() const;
            bool isDownPressed() const;
            bool isLeftPressed() const;
            bool isRightPressed() const;
            bool isShootPressed() const;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            int m_lastAppliedSkinIndex = -1;
    }; // class GameSolo
} // namespace cli