///
/// @file PlayerControllerMulti.hpp
/// @brief Multiplayer player controller system for R-Type client
/// @details This file contains the player controller system for multiplayer mode.
///          It handles local player input, sends input commands to the server,
///          and manages client-side prediction for smooth gameplay.
///          The system implements input throttling and sequence numbering for
///          reliable input transmission over the network.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include <deque>

#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Common.hpp"
#include "Utils/EventBus.hpp"

namespace gme
{
    ///
    /// @class PlayerControllerMulti
    /// @brief Manages local player input and network communication for multiplayer
    /// @details This system handles:
    ///          - Capturing and processing player keyboard input
    ///          - Sending input commands to the game server
    ///          - Client-side prediction for responsive controls
    ///          - Input throttling to reduce network bandwidth
    ///          - Sequence numbering for input acknowledgment
    ///
    /// The controller sends input updates to the server at a maximum rate
    /// of 144 Hz and maintains a history of pending inputs for reconciliation.
    ///
    /// @namespace gme
    ///
    class PlayerControllerMulti final : public ecs::ASystem
    {
        public:
            ///
            /// @brief Constructor
            /// @param renderer Shared pointer to the renderer interface
            /// @param sessionId Player's network session ID
            /// @details Initializes the controller and registers with the event bus
            ///
            explicit PlayerControllerMulti(const std::shared_ptr<eng::IRenderer> &renderer, uint32_t sessionId)
                : m_renderer(renderer), m_playerEntity(0), m_eventBus(utl::EventBus::getInstance())
            {
                m_componentId = 11;
                m_eventBus.registerComponent(m_componentId, "PlayerControllerMulti");
            }

            ///
            /// @brief Destructor
            ///
            ~PlayerControllerMulti() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            PlayerControllerMulti(const PlayerControllerMulti &) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            PlayerControllerMulti &operator=(const PlayerControllerMulti &) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            PlayerControllerMulti(PlayerControllerMulti &&) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            PlayerControllerMulti &operator=(PlayerControllerMulti &&) = delete;

            ///
            /// @brief Update the player controller system (called each frame)
            /// @param registry ECS registry containing all entities and components
            /// @param dt Delta time since last frame (in seconds)
            /// @details Processes input state and sends updates to server if inputs changed
            ///
            void update(ecs::Registry &registry, float dt) override;

            ///
            /// @brief Handle input events from the window
            /// @param registry ECS registry
            /// @param event Input event (keyboard press/release)
            /// @details Updates internal key state for the player's controls
            ///
            void handleInput(ecs::Registry &registry, const eng::Event &event);

            ///
            /// @brief Create the local player entity
            /// @param registry ECS registry to create entity in
            /// @param x Initial X position
            /// @param y Initial Y position
            /// @return Created player entity ID
            /// @details Creates a player entity with all necessary components for gameplay
            ///
            ecs::Entity createPlayer(ecs::Registry &registry, float x, float y);

            ///
            /// @brief Check if space bar is currently pressed
            /// @return True if space is pressed (shooting)
            ///
            bool isSpacePressed() const;

        private:
            ///
            /// @brief Send input command to server via event bus
            /// @param up Move up key pressed
            /// @param down Move down key pressed
            /// @param left Move left key pressed
            /// @param right Move right key pressed
            /// @param shoot Shoot key pressed
            /// @details Serializes input state and sends to server as INPUT event
            ///
            void sendInputToServer(bool up, bool down, bool left, bool right, bool shoot) const;

            ///
            /// @brief Send input update if state has changed
            /// @details Only sends updates when input state changes to reduce network traffic
            ///
            void sendInputsIfChanged();

            const std::shared_ptr<eng::IRenderer> &m_renderer; ///< Renderer interface reference
            std::unordered_map<eng::Key, bool> m_keysPressed;  ///< Current state of all keys
            ecs::Entity m_playerEntity;                        ///< Local player entity ID
            uint32_t m_componentId;                            ///< Event bus component ID
            utl::EventBus &m_eventBus;                         ///< Event bus reference

            const float INPUT_THROTTLE_INTERVAL = 1.0f / 144.0f; ///< Maximum input send rate (144 Hz)

            uint32_t m_nextSeqId = 1;    ///< Next sequence ID for input packets
            uint32_t m_lastAckSeqId = 0; ///< Last acknowledged sequence ID from server

            ///
            /// @struct PendingInput
            /// @brief Represents an input command awaiting server acknowledgment
            /// @details Used for client-side prediction and reconciliation
            ///
            struct PendingInput
            {
                    uint32_t seqId;                 ///< Sequence number of this input
                    std::vector<uint8_t> inputData; ///< Serialized input data
                    float dt;                       ///< Delta time when input was sent
            };
    }; // class PlayerControllerMulti
} // namespace gme
