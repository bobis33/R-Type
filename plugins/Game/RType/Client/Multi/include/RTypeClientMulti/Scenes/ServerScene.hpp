///
/// @file ServerScene.hpp
/// @brief Server connection configuration scene for R-Type multiplayer
/// @details This file contains the scene where players configure their connection settings
///          before joining a multiplayer game. Players can set their name, server IP,
///          and port before connecting to the game server.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "Engine/Interfaces/IScene.hpp"

namespace gme
{
    ///
    /// @class ServerScene
    /// @brief Scene for configuring and establishing server connection
    /// @details This scene provides a user interface for players to:
    ///          - Enter their player name
    ///          - Specify server IP address
    ///          - Set server port number
    ///          - Connect to the game server
    ///          - Navigate back to the main menu
    ///
    /// The scene uses a menu-based navigation system with keyboard input
    /// and visual feedback for the selected option.
    ///
    /// @namespace gme
    ///
    class ServerScene final : public eng::AScene
    {
        public:
            ///
            /// @brief Constructor
            /// @param assignedId Scene identifier
            /// @param renderer Shared pointer to the renderer interface
            /// @param host Default server host/IP address
            /// @param port Default server port
            /// @param playerName Default player name
            /// @details Initializes the scene with default connection parameters
            ///
            ServerScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer, const std::string &host,
                        const std::string &port, const std::string &playerName);

            ///
            /// @brief Destructor
            ///
            ~ServerScene() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            ServerScene(const ServerScene &other) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            ServerScene &operator=(const ServerScene &other) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            ServerScene(ServerScene &&other) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            ServerScene &operator=(ServerScene &&other) = delete;

            ///
            /// @brief Update the scene (called each frame)
            /// @param dt Delta time since last frame
            /// @param size Current window size
            /// @details Handles animation updates and UI state changes
            ///
            void update(float dt, const eng::WindowSize &size) override;

            ///
            /// @brief Handle input events
            /// @param event Input event (keyboard, mouse, etc.)
            /// @details Processes user input for menu navigation and text entry
            ///
            void event(const eng::Event &event) override;

            ///
            /// @brief Get music playback state
            /// @return Reference to music playback flag
            ///
            bool &playMusic() { return m_playMusic; }

            ///
            /// @brief Callback triggered when player attempts to connect
            /// @details Function signature: (playerName, serverIP, serverPort) -> void
            ///
            std::function<void(const std::string &playerName, const std::string &serverIP,
                               const std::string &serverPort)>
                onConnect;

            ///
            /// @brief Callback triggered when player returns to main menu
            ///
            std::function<void()> onBackToMenu;

        private:
            size_t m_selectedIndex = 0;   ///< Currently selected menu option index
            float m_animationTime = 0.0f; ///< Animation timer for visual effects

            ///< Menu options displayed to the player
            const std::vector<std::string> m_serverOptions = {"Player Name", "Server IP", "Server Port", "Connect",
                                                              "Back"};

            ///
            /// @brief Initiate server connection with provided settings
            /// @param playerName Player's display name
            /// @param serverIP Server IP address or hostname
            /// @param serverPort Server port number as string
            /// @details Validates inputs and triggers the onConnect callback
            ///
            void connectServer(const std::string &playerName, const std::string &serverIP,
                               const std::string &serverPort) const;

            std::string m_playerName; ///< Player name input field
            std::string m_host;       ///< Server host/IP input field
            std::string m_port;       ///< Server port input field

            ecs::Entity m_playerNameValueEntity = 0; ///< Text entity displaying player name
            ecs::Entity m_serverIPValueEntity = 0;   ///< Text entity displaying server IP
            ecs::Entity m_serverPortValueEntity = 0; ///< Text entity displaying server port
            bool m_playMusic = false;                ///< Music playback state flag

            ///
            /// @brief Get reference to the currently selected input field
            /// @return Reference to the string being edited
            ///
            std::string &getCurrentEditField();

            ///
            /// @brief Update the visual display of input values
            /// @details Refreshes text entities to reflect current input field values
            ///
            void updateValueDisplay();
    }; // class ServerScene
} // namespace gme
