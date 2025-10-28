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
#include "RTypeClientMulti/Managers/StageManager.hpp"
#include "Utils/EventBus.hpp"

namespace gme
{
    class PlayerControllerMulti;
} // namespace gme

namespace gme
{
    struct InterpolationData
    {
            float targetX, targetY;
            float targetVx, targetVy;
            float currentX, currentY;
            float smoothFactor;
            float targetRotation;
            float currentRotation;
    };

    ///
    /// @class GameMulti
    /// @brief GameMulti scene
    /// @namespace gme
    ///
    class GameMulti final : public eng::AScene
    {
        public:
            GameMulti(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                      const std::shared_ptr<eng::IAudio> &audio, float skinIndex, bool &showDebug, uint32_t sessionId);
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
            void setupEventSubscriptions() const;
            void processEventBus();
            void handlePlayerInputReceived(const utl::Event &event);
            void handleWorldStateUpdate(const utl::Event &event);

            void updateInterpolation(std::unordered_map<uint32_t, InterpolationData> &dataMap,
                                     std::unordered_map<uint32_t, ecs::Entity> &entityMap, float smoothFactor, float dt,
                                     ecs::Registry &registry);

            ecs::Entity m_localPlayerEntity;
            std::unordered_map<uint32_t, ecs::Entity> m_remotePlayers;

            const std::shared_ptr<eng::IAudio> &m_audio;
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            float m_skinIndex;
            std::unique_ptr<PlayerControllerMulti> m_playerController;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            bool &m_showDebug;
            bool m_playMusic = false;

            uint32_t m_sessionId;
            uint32_t m_eventComponentId = 10;
            std::unique_ptr<StageManager> m_stageManager;

            std::unordered_map<uint32_t, ecs::Entity> m_projectileEntities;
            std::unordered_map<uint32_t, ecs::Entity> m_enemyEntities;

            std::unordered_map<uint32_t, InterpolationData> m_remotePlayerData;
            std::unordered_map<uint32_t, InterpolationData> m_projectileData;
            std::unordered_map<uint32_t, InterpolationData> m_enemyData;
            std::unordered_map<uint32_t, uint32_t> m_playerSkinMap;
            struct PendingInput
            {
                    uint32_t seqId;
                    std::vector<uint8_t> inputData;
                    float dt;
            };
            std::deque<PendingInput> m_inputHistory;
            uint32_t m_lastAckSeqId = 0;

            static constexpr float REMOTE_PLAYER_SMOOTH_FACTOR = 0.25f;
            static constexpr float PROJECTILE_SMOOTH_FACTOR = 0.5f;
            static constexpr float ENEMY_SMOOTH_FACTOR = 0.18f;

            bool m_starfieldCreated = false;
    }; // class GameMulti
} // namespace gme
