///
/// @file GameSolo.hpp
/// @brief This file contains the solo Game scene
/// @namespace gme
///

#pragma once

#include <memory>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientSolo/Managers/StageManager.hpp"

namespace gme
{
    class PlayerController;
} // namespace gme

namespace gme
{
    ///
    /// @class GameSolo
    /// @brief GameSolo scene
    /// @namespace gme
    ///
    class GameSolo final : public eng::AScene
    {
        public:
            GameSolo(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                     const std::shared_ptr<eng::IAudio> &audio, int skinIndex, bool &showDebug);
            ~GameSolo() override = default;

            GameSolo(const GameSolo &other) = delete;
            GameSolo &operator=(const GameSolo &other) = delete;
            GameSolo(GameSolo &&other) = delete;
            GameSolo &operator=(GameSolo &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            void updatePlayerSkin();

        private:
            void handlePlayerInputs(ecs::Registry &registry, float dt);
            static ecs::Entity createPlayer(ecs::Registry &registry);

            const std::shared_ptr<eng::IAudio> &m_audio;
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_playerEntity;
            int m_skinIndex;
            int m_lastAppliedSkinIndex = -1;
            bool &m_showDebug;
            std::unique_ptr<StageManager> m_stageManager;
    }; // class GameSolo
} // namespace gme