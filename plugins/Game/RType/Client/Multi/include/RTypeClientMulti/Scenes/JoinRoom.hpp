///
/// @file JoinRoom.hpp
/// @brief This file contains the join room scene
/// @namespace gme
///

#pragma once

#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace gme
{
    ///
    /// @struct RoomInfo
    /// @brief Structure containing room information
    ///
    struct RoomInfo
    {
            std::string name;
            int currentPlayers;
            int maxPlayers;
            int roomId;
    };

    ///
    /// @class JoinRoomScene
    /// @brief Join room scene
    /// @namespace gme
    ///
    class JoinRoomScene final : public eng::AScene
    {
        public:
            JoinRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                          const std::shared_ptr<eng::IAudio> &audio);
            ~JoinRoomScene() override = default;

            JoinRoomScene(const JoinRoomScene &other) = delete;
            JoinRoomScene &operator=(const JoinRoomScene &other) = delete;
            JoinRoomScene(JoinRoomScene &&other) = delete;
            JoinRoomScene &operator=(JoinRoomScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            void setRooms(const std::vector<RoomInfo> &rooms);
            void refreshRoomList();

            std::function<void(int roomId)> onJoin;
            std::function<void()> onBackToMulti;
            std::function<void()> onRefreshRequest;

        private:
            const std::shared_ptr<eng::IAudio> &m_audio;

            size_t m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            std::vector<RoomInfo> m_rooms;

            ecs::Entity m_titleEntity = 0;
            ecs::Entity m_noRoomsEntity = 0;
            ecs::Entity m_refreshButtonEntity = 0;
            ecs::Entity m_backButtonEntity = 0;
            std::vector<ecs::Entity> m_roomEntities;

            void updateRoomDisplay();
            void clearRoomEntities();
    }; // class JoinRoomScene
} // namespace gme
