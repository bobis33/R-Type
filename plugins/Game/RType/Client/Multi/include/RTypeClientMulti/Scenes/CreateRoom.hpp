///
/// @file CreateRoom.hpp
/// @brief This file contains the create room scene
/// @namespace gme
///

#pragma once

#include "Engine/Interfaces/IScene.hpp"

namespace gme
{
    ///
    /// @class CreateRoomScene
    /// @brief Create room scene
    /// @namespace gme
    ///
    class CreateRoomScene final : public eng::AScene
    {
        public:
            CreateRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~CreateRoomScene() override = default;

            CreateRoomScene(const CreateRoomScene &other) = delete;
            CreateRoomScene &operator=(const CreateRoomScene &other) = delete;
            CreateRoomScene(CreateRoomScene &&other) = delete;
            CreateRoomScene &operator=(CreateRoomScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            void processEventBus() const;
            bool& playMusic() { return m_playMusic; }

            std::function<void(const std::string &roomName, int maxPlayers)> onCreate;
            std::function<void()> onBackToMulti;
            std::function<void(int lobbyId, const rnp::LobbyInfo *lobbyInfo)> onRoomCreated;

        private:
            size_t m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            const std::vector<std::string> m_options = {"Room Name", "Max Players", "Create", "Back"};
        bool m_playMusic = false;
            std::string m_roomName = "My Room";
            int m_maxPlayers = 4;

            ecs::Entity m_titleEntity = 0;
            ecs::Entity m_roomNameValueEntity = 0;
            ecs::Entity m_maxPlayersValueEntity = 0;

            void updateValueDisplay();
            void createRoom() const;
    }; // class CreateRoomScene
} // namespace gme
