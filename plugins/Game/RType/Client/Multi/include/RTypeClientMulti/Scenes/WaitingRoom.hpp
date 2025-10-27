///
/// @file WaitingRoom.hpp
/// @brief This file contains the waiting room scene
/// @namespace gme
///

#pragma once

#include <functional>
#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/Protocol/Protocol.hpp"

namespace gme
{
    ///
    /// @class WaitingRoomScene
    /// @brief Waiting room scene where players wait before game starts
    /// @namespace gme
    ///
    class WaitingRoomScene final : public eng::AScene
    {
        public:
            WaitingRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~WaitingRoomScene() override = default;

            WaitingRoomScene(const WaitingRoomScene &other) = delete;
            WaitingRoomScene &operator=(const WaitingRoomScene &other) = delete;
            WaitingRoomScene(WaitingRoomScene &&other) = delete;
            WaitingRoomScene &operator=(WaitingRoomScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            void setLobbyId(std::uint32_t lobbyId);
            void setLobbyInfo(const rnp::LobbyInfo &lobbyInfo);
            void setIsHost(const bool isHost) { m_isHost = isHost; }
            [[nodiscard]] bool isHost() const { return m_isHost; }
            std::uint32_t getLobbyId() const { return m_lobbyId; };

            bool &playMusic() { return m_playMusic; }

            std::function<void()> onLeaveLobby;
            std::function<void()> onGameStart;
            void startGame();

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;

            std::uint32_t m_lobbyId = 0;
            rnp::LobbyInfo m_currentLobbyInfo;
            bool m_hasLobbyInfo = false;
            float m_animationTime = 0.0f;

            int m_selectedButton = 0;
            static constexpr int BUTTON_LEAVE = 0;
            static constexpr int BUTTON_READY = 1;
            static constexpr int BUTTON_START = 2;
            static constexpr int BUTTON_COUNT = 3;

            // UI Entities
            ecs::Entity m_lobbyIdEntity = 0;
            ecs::Entity m_playerCountEntity = 0;
            ecs::Entity m_statusEntity = 0;
            ecs::Entity m_leaveButtonEntity = 0;
            ecs::Entity m_startButtonEntity = 0;
            ecs::Entity m_readyButtonEntity = 0;
            std::vector<ecs::Entity> m_playerEntities;

            bool m_isHost = false;
            bool m_playMusic = false;
            void setupEventSubscriptions() const;
            void processEventBus();
            void handleLobbyUpdate(const utl::Event &event);
            void handleGameStart(const utl::Event &event) const;
            void updatePlayerDisplay();
            void clearPlayerEntities();
            void leaveLobby() const;

    }; // class WaitingRoomScene
} // namespace gme
