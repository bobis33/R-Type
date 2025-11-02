///
/// @file WaitingRoom.hpp
/// @brief Waiting room scene for R-Type multiplayer lobby system
/// @details This file contains the waiting room scene where players gather before a game starts.
///          Players can see other lobby members, wait for the host to start the game,
///          and leave the lobby if desired. The scene displays real-time lobby updates
///          and provides controls for lobby management.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include <functional>
#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/Protocol/Protocol.hpp"

namespace gme
{
    ///
    /// @struct InterpolationData
    /// @brief Data structure for smooth entity interpolation (forward declaration)
    ///

    ///
    /// @class WaitingRoomScene
    /// @brief Pre-game lobby scene where players wait before match starts
    /// @details This scene provides:
    ///          - Real-time display of lobby information (name, player count, status)
    ///          - List of connected players in the lobby
    ///          - Host controls (start game button)
    ///          - Player controls (ready button, leave button)
    ///          - Animated UI elements for visual feedback
    ///          - Event bus integration for network updates
    ///
    /// The scene automatically updates when lobby state changes (players join/leave)
    /// and transitions to the game scene when the host starts the game.
    ///
    /// @namespace gme
    ///
    class WaitingRoomScene final : public eng::AScene
    {
        public:
            ///
            /// @brief Constructor
            /// @param assignedId Unique scene identifier
            /// @param renderer Shared pointer to the rendering interface
            /// @details Initializes the waiting room UI and sets up event subscriptions
            ///
            WaitingRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);

            ///
            /// @brief Destructor
            ///
            ~WaitingRoomScene() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            WaitingRoomScene(const WaitingRoomScene &other) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            WaitingRoomScene &operator=(const WaitingRoomScene &other) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            WaitingRoomScene(WaitingRoomScene &&other) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            WaitingRoomScene &operator=(WaitingRoomScene &&other) = delete;

            ///
            /// @brief Update the waiting room scene (called each frame)
            /// @param dt Delta time since last frame (in seconds)
            /// @param size Current window dimensions
            /// @details Updates animations, processes event bus messages, and refreshes UI
            ///
            void update(float dt, const eng::WindowSize &size) override;

            ///
            /// @brief Handle input events
            /// @param event Input event (keyboard, mouse)
            /// @details Handles navigation between buttons and activation of controls
            ///
            void event(const eng::Event &event) override;

            ///
            /// @brief Set the lobby ID
            /// @param lobbyId Unique lobby identifier
            ///
            void setLobbyId(std::uint32_t lobbyId);

            ///
            /// @brief Update lobby information
            /// @param lobbyInfo Current lobby state from server
            /// @details Updates the displayed lobby information and player list
            ///
            void setLobbyInfo(const rnp::LobbyInfo &lobbyInfo);

            ///
            /// @brief Set whether local player is the lobby host
            /// @param isHost True if local player is host
            ///
            void setIsHost(const bool isHost) { m_isHost = isHost; }

            ///
            /// @brief Check if local player is the lobby host
            /// @return True if local player is host
            ///
            [[nodiscard]] bool isHost() const { return m_isHost; }

            ///
            /// @brief Get current lobby ID
            /// @return Lobby identifier
            ///
            std::uint32_t getLobbyId() const { return m_lobbyId; };

            ///
            /// @brief Get reference to music playback flag
            /// @return Reference to music play state
            ///
            bool &playMusic() { return m_playMusic; }

            ///
            /// @brief Callback invoked when player leaves lobby
            ///
            std::function<void()> onLeaveLobby;

            ///
            /// @brief Callback invoked when game starts
            ///
            std::function<void()> onGameStart;

            ///
            /// @brief Initiate game start sequence
            /// @details Called when host presses start button
            ///
            void startGame();

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer; ///< Renderer interface reference

            std::uint32_t m_lobbyId = 0;         ///< Current lobby ID
            rnp::LobbyInfo m_currentLobbyInfo{}; ///< Current lobby state
            bool m_hasLobbyInfo = false;         ///< Whether lobby info has been received
            float m_animationTime = 0.0f;        ///< Timer for UI animations

            int m_selectedButton = 0;              ///< Currently selected button index
            static constexpr int BUTTON_LEAVE = 0; ///< Leave lobby button index
            static constexpr int BUTTON_READY = 1; ///< Ready button index
            static constexpr int BUTTON_START = 2; ///< Start game button index (host only)
            static constexpr int BUTTON_COUNT = 3; ///< Total number of buttons

            // UI Entity IDs
            ecs::Entity m_lobbyIdEntity = 0;           ///< Entity displaying lobby ID
            ecs::Entity m_playerCountEntity = 0;       ///< Entity displaying player count
            ecs::Entity m_statusEntity = 0;            ///< Entity displaying lobby status
            ecs::Entity m_leaveButtonEntity = 0;       ///< Leave button entity
            ecs::Entity m_startButtonEntity = 0;       ///< Start game button entity (host only)
            ecs::Entity m_readyButtonEntity = 0;       ///< Ready button entity
            std::vector<ecs::Entity> m_playerEntities; ///< Entities for player name displays

            bool m_isHost = false;    ///< Whether local player is lobby host
            bool m_playMusic = false; ///< Music playback state

            ///
            /// @brief Subscribe to event bus events
            /// @details Registers for LOBBY_UPDATE and GAME_START events
            ///
            void setupEventSubscriptions() const;

            ///
            /// @brief Process events from event bus
            /// @details Polls and handles network events
            ///
            void processEventBus();

            ///
            /// @brief Handle lobby update event from server
            /// @param event Event containing updated lobby information
            ///
            void handleLobbyUpdate(const utl::Event &event);

            ///
            /// @brief Handle game start event from server
            /// @param event Event indicating game is starting
            ///
            void handleGameStart(const utl::Event &event) const;

            ///
            /// @brief Update the player list display
            /// @details Recreates player name entities based on current lobby info
            ///
            void updatePlayerDisplay();

            ///
            /// @brief Clear all player display entities
            ///
            void clearPlayerEntities();

            ///
            /// @brief Send leave lobby request to server
            ///
            void leaveLobby() const;

    }; // class WaitingRoomScene
} // namespace gme
