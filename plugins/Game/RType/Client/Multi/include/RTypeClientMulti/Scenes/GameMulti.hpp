///
/// @file GameMulti.hpp
/// @brief This file contains the multiplayer Game scene
/// @namespace gme
///

#pragma once

#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Utils/EventBus.hpp"

namespace gme
{
    class PlayerControllerMulti;
} // namespace gme

namespace gme
{
    ///
    /// @class GameMulti
    /// @brief GameMulti scene
    /// @namespace gme
    ///
    class GameMulti final : public eng::AScene
    {
        public:
            GameMulti(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                      const std::shared_ptr<eng::IAudio> &audio, const float skinIndex, bool &showDebug,
                      const uint32_t lobbyId, const uint32_t sessionId);
            ~GameMulti() override = default;

            GameMulti(const GameMulti &other) = delete;
            GameMulti &operator=(const GameMulti &other) = delete;
            GameMulti(GameMulti &&other) = delete;
            GameMulti &operator=(GameMulti &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            void updatePlayerSkin();

            bool &playMusic() { return m_playMusic; }

        private:
            void setupEventSubscriptions();
            void processEventBus();
            void handlePlayerInputReceived(const utl::Event &event);
            void handleWorldStateUpdate(const utl::Event &event);

            ecs::Entity m_localPlayerEntity;
            std::unordered_map<uint32_t, ecs::Entity> m_remotePlayers;

            const std::shared_ptr<eng::IAudio> &m_audio;
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            float m_skinIndex;
            std::unique_ptr<PlayerControllerMulti> m_playerController;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            int m_lastAppliedSkinIndex = -1;
            bool &m_showDebug;
            bool m_playMusic = false;

            uint32_t m_lobbyId;
            uint32_t m_sessionId;
            uint32_t m_eventComponentId = 10;

            std::unordered_map<uint32_t, ecs::Entity> m_projectileEntities;

            // Local player input tracking for prediction + reconciliation
            struct PendingInput
            {
                    uint32_t seqId;
                    std::vector<uint8_t> inputData; // [up, down, left, right, shoot]
                    float dt;
            };
            std::deque<PendingInput> m_inputHistory;
            uint32_t m_nextSeqId = 1;
            uint32_t m_lastAckSeqId = 0;

            // Interpolation data for smooth remote player movement
            struct InterpolationData
            {
                    float prevX, prevY;
                    float targetX, targetY;
                    float interpolationTime;
                    float interpolationDuration;
            };
            std::unordered_map<uint32_t, InterpolationData> m_interpolationData;

            bool m_starfieldCreated = false;
    }; // class GameMulti
} // namespace gme
