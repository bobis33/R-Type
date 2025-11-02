///
/// @file CreateRoom.hpp
/// @brief Lobby creation scene for R-Type multiplayer
/// @details This file contains the scene where players create new game lobbies.
///          Players can configure lobby settings including name and maximum player count
///          before creating the room on the server. The scene provides a user-friendly
///          interface for lobby customization.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "Engine/Interfaces/IScene.hpp"

namespace gme
{
    ///
    /// @class CreateRoomScene
    /// @brief Scene for creating and configuring new multiplayer lobbies
    /// @details This scene provides an interface for players to:
    ///          - Enter a custom lobby/room name
    ///          - Set maximum player count (2-8 players)
    ///          - Create the lobby on the server
    ///          - Navigate back to multiplayer menu
    ///
    /// The scene uses a menu-based navigation system with keyboard input
    /// and provides visual feedback for the selected option. Upon successful
    /// lobby creation, the scene transitions to the waiting room.
    ///
    /// @namespace gme
    ///
    class CreateRoomScene final : public eng::AScene
    {
        public:
            ///
            /// @brief Constructor
            /// @param assignedId Unique scene identifier
            /// @param renderer Shared pointer to the rendering interface
            /// @details Initializes the create room UI with default values
            ///
            CreateRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);

            ///
            /// @brief Destructor
            ///
            ~CreateRoomScene() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            CreateRoomScene(const CreateRoomScene &other) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            CreateRoomScene &operator=(const CreateRoomScene &other) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            CreateRoomScene(CreateRoomScene &&other) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            CreateRoomScene &operator=(CreateRoomScene &&other) = delete;

            ///
            /// @brief Update the scene (called each frame)
            /// @param dt Delta time since last frame (in seconds)
            /// @param size Current window dimensions
            /// @details Updates animations and UI state
            ///
            void update(float dt, const eng::WindowSize &size) override;

            ///
            /// @brief Handle input events
            /// @param event Input event (keyboard, mouse)
            /// @details Processes user input for menu navigation and value modification
            ///
            void event(const eng::Event &event) override;

            ///
            /// @brief Process events from the event bus
            /// @details Handles lobby creation responses from the network layer
            ///
            void processEventBus() const;

            ///
            /// @brief Get reference to music playback flag
            /// @return Reference to music play state
            ///
            bool &playMusic() { return m_playMusic; }

            ///
            /// @brief Callback invoked when player creates a room
            /// @details Function signature: (roomName, maxPlayers) -> void
            ///
            std::function<void(const std::string &roomName, int maxPlayers)> onCreate;

            ///
            /// @brief Callback invoked when player navigates back to multiplayer menu
            ///
            std::function<void()> onBackToMulti;

            ///
            /// @brief Callback invoked when room is successfully created on server
            /// @details Function signature: (lobbyId, lobbyInfo*) -> void
            ///
            std::function<void(int lobbyId, const rnp::LobbyInfo *lobbyInfo)> onRoomCreated;

        private:
            size_t m_selectedIndex = 0;   ///< Currently selected menu option index
            float m_animationTime = 0.0f; ///< Animation timer for visual effects

            ///< Menu options displayed to the player
            const std::vector<std::string> m_options = {"Room Name", "Max Players", "Create", "Back"};

            bool m_playMusic = false;           ///< Music playback state flag
            std::string m_roomName = "My Room"; ///< Current room name input (default: "My Room")
            int m_maxPlayers = 4;               ///< Maximum players setting (default: 4)

            ecs::Entity m_titleEntity = 0;           ///< Title text entity
            ecs::Entity m_roomNameValueEntity = 0;   ///< Text entity displaying room name
            ecs::Entity m_maxPlayersValueEntity = 0; ///< Text entity displaying max players

            ///
            /// @brief Update the visual display of configuration values
            /// @details Refreshes text entities to reflect current room name and max players
            ///
            void updateValueDisplay();

            ///
            /// @brief Send lobby creation request to server
            /// @details Validates inputs and triggers the onCreate callback
            ///
            void createRoom() const;
    }; // class CreateRoomScene
} // namespace gme
