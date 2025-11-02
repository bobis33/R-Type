///
/// @file JoinRoom.hpp
/// @brief Lobby browser scene for joining multiplayer games
/// @details This file contains the join room scene where players can browse available
///          lobbies, view lobby information (name, player count, status), and join
///          a game. The scene provides real-time updates via the event bus and
///          supports refreshing the lobby list on demand.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/Protocol/Protocol.hpp"

namespace gme
{
    ///
    /// @class JoinRoomScene
    /// @brief Scene for browsing and joining available multiplayer lobbies
    /// @details This scene provides:
    ///          - Display of all available lobbies with their information
    ///          - Real-time lobby list updates via event bus
    ///          - Manual refresh capability
    ///          - Lobby selection and join functionality
    ///          - Navigation back to multiplayer menu
    ///          - Visual feedback for lobby status and player counts
    ///
    /// The scene automatically subscribes to lobby list updates and join responses
    /// from the network layer, providing a responsive lobby browser experience.
    ///
    /// @namespace gme
    ///
    class JoinRoomScene final : public eng::AScene
    {
        public:
            ///
            /// @brief Constructor
            /// @param assignedId Unique scene identifier
            /// @param renderer Shared pointer to the rendering interface
            /// @details Initializes the lobby browser UI and sets up event subscriptions
            ///
            JoinRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);

            ///
            /// @brief Destructor
            ///
            ~JoinRoomScene() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            JoinRoomScene(const JoinRoomScene &other) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            JoinRoomScene &operator=(const JoinRoomScene &other) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            JoinRoomScene(JoinRoomScene &&other) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            JoinRoomScene &operator=(JoinRoomScene &&other) = delete;

            ///
            /// @brief Update the join room scene (called each frame)
            /// @param dt Delta time since last frame (in seconds)
            /// @param size Current window dimensions
            /// @details Processes event bus messages, updates animations, and refreshes UI
            ///
            void update(float dt, const eng::WindowSize &size) override;

            ///
            /// @brief Handle input events
            /// @param event Input event (keyboard, mouse)
            /// @details Handles navigation between lobbies and selection/refresh actions
            ///
            void event(const eng::Event &event) override;

            ///
            /// @brief Get reference to music playback flag
            /// @return Reference to music play state
            ///
            bool &playMusic() { return m_playMusic; }

            ///
            /// @brief Update the displayed list of available lobbies
            /// @param rooms Vector of lobby information structures from server
            /// @details Refreshes the lobby list display with new data
            ///
            void setRooms(const std::vector<rnp::LobbyInfo> &rooms);

            ///
            /// @brief Request updated lobby list from server
            /// @details Sends LOBBY_LIST_REQUEST to the network layer
            ///
            void refreshRoomList() const;

            ///
            /// @brief Callback invoked when player joins a lobby
            /// @details Function signature: (roomId, lobbyInfo*) -> void
            ///
            std::function<void(int roomId, const rnp::LobbyInfo *lobbyInfo)> onJoin;

            ///
            /// @brief Callback invoked when returning to multiplayer menu
            ///
            std::function<void()> onBackToMulti;

            ///
            /// @brief Callback invoked when refresh is requested
            ///
            std::function<void()> onRefreshRequest;

        private:
            size_t m_selectedIndex = 0;          ///< Currently selected lobby index
            float m_animationTime = 0.0f;        ///< Animation timer for visual effects
            std::vector<rnp::LobbyInfo> m_rooms; ///< List of available lobbies from server

            ecs::Entity m_noRoomsEntity = 0;         ///< Entity displaying "No rooms available" message
            std::vector<ecs::Entity> m_roomEntities; ///< Entities for lobby list items
            bool m_playMusic = false;                ///< Music playback state flag

            ///
            /// @brief Update the visual display of the lobby list
            /// @details Recreates lobby list entities based on current room data
            ///
            void updateRoomDisplay();

            ///
            /// @brief Clear all lobby display entities
            /// @details Removes all lobby list item entities from the scene
            ///
            void clearRoomEntities();

            ///
            /// @brief Process events from event bus
            /// @details Polls and handles network events (lobby list, join response)
            ///
            void processEventBus();

            ///
            /// @brief Subscribe to event bus events
            /// @details Registers for LOBBY_LIST_RESPONSE and LOBBY_JOIN_RESPONSE events
            ///
            void setupEventSubscriptions() const;

            ///
            /// @brief Handle lobby list response from server
            /// @param event Event containing list of available lobbies
            ///
            void handleLobbyListResponse(const utl::Event &event);

            ///
            /// @brief Handle lobby join response from server
            /// @param event Event containing join result (success/failure)
            ///
            void handleLobbyJoinResponse(const utl::Event &event) const;
    }; // class JoinRoomScene
} // namespace gme
