///
/// @file JoinRoom.hpp
/// @brief This file contains the join room scene
/// @namespace gme
///

#pragma once

#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/Protocol/Protocol.hpp"

namespace gme
{
    ///
    /// @class JoinRoomScene
    /// @brief Join room scene
    /// @namespace gme
    ///
    class JoinRoomScene final : public eng::AScene
    {
        public:
            JoinRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~JoinRoomScene() override = default;

            JoinRoomScene(const JoinRoomScene &other) = delete;
            JoinRoomScene &operator=(const JoinRoomScene &other) = delete;
            JoinRoomScene(JoinRoomScene &&other) = delete;
            JoinRoomScene &operator=(JoinRoomScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            bool &playMusic() { return m_playMusic; }

            void setRooms(const std::vector<rnp::LobbyInfo> &rooms);
            void refreshRoomList() const;

            std::function<void(int roomId, const rnp::LobbyInfo *lobbyInfo)> onJoin;
            std::function<void()> onBackToMulti;
            std::function<void()> onRefreshRequest;

        private:
            size_t m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            std::vector<rnp::LobbyInfo> m_rooms;

            ecs::Entity m_noRoomsEntity = 0;
            std::vector<ecs::Entity> m_roomEntities;
            bool m_playMusic = false;
            void updateRoomDisplay();
            void clearRoomEntities();
            void processEventBus();
            void setupEventSubscriptions() const;
            void handleLobbyListResponse(const utl::Event &event);
            void handleLobbyJoinResponse(const utl::Event &event) const;
    }; // class JoinRoomScene
} // namespace gme
