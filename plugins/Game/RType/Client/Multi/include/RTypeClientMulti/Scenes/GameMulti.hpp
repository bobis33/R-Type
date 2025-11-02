///
/// @file GameMulti.hpp
/// @brief Main multiplayer game scene for R-Type
/// @details This file contains the primary gameplay scene for R-Type multiplayer mode.
///          It manages game state synchronization with the server, entity interpolation,
///          player controls, HUD rendering, and game logic. The scene handles real-time
///          networking updates and provides smooth gameplay through client-side prediction
///          and interpolation techniques.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientMulti/Managers/StageManager.hpp"
#include "RTypeClientMulti/Systems/HUD.hpp"
#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "Utils/EventBus.hpp"

namespace gme
{
    ///
    /// @struct InterpolationData
    /// @brief Data structure for entity position interpolation
    /// @details Stores target and current position/velocity data for smooth entity movement.
    ///          Used to interpolate entity positions between server updates to provide
    ///          smooth visual rendering despite network latency.
    ///
    struct InterpolationData
    {
            float targetX, targetY;   ///< Target position from server update
            float targetVx, targetVy; ///< Target velocity from server
            float currentX, currentY; ///< Current interpolated position
            float smoothFactor;       ///< Interpolation smoothing factor (0.0-1.0)
            float targetRotation;     ///< Target rotation angle
            float currentRotation;    ///< Current interpolated rotation
    };

    ///
    /// @class GameMulti
    /// @brief Main multiplayer game scene handling networked gameplay
    /// @details This scene manages the core multiplayer game loop including:
    ///          - Network synchronization with game server
    ///          - Local and remote player entity management
    ///          - Entity interpolation for smooth rendering
    ///          - Client-side prediction for responsive controls
    ///          - Enemy and projectile synchronization
    ///          - HUD and game state display
    ///          - Audio management (music and sound effects)
    ///          - Stage progression
    ///
    /// The scene uses event bus communication to receive world state updates from
    /// the network client and applies interpolation to create smooth visuals despite
    /// network latency and packet timing.
    ///
    /// @namespace gme
    ///
    class GameMulti final : public eng::AScene
    {
        public:
            ///
            /// @brief Constructor
            /// @param assignedId Scene ID assigned by the engine
            /// @param renderer Shared pointer to the renderer interface
            /// @param audio Shared pointer to the audio interface
            /// @param skinIndex Player skin/appearance index
            /// @param showDebug Reference to debug mode flag
            /// @param sessionId Network session ID of the local player
            ///
            GameMulti(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                      const std::shared_ptr<eng::IAudio> &audio, float skinIndex, bool &showDebug, uint32_t sessionId);

            ///
            /// @brief Destructor
            ///
            ~GameMulti() override = default;

            /// @brief Deleted copy constructor (non-copyable)
            GameMulti(const GameMulti &other) = delete;
            /// @brief Deleted copy assignment operator (non-copyable)
            GameMulti &operator=(const GameMulti &other) = delete;
            /// @brief Deleted move constructor (non-movable)
            GameMulti(GameMulti &&other) = delete;
            /// @brief Deleted move assignment operator (non-movable)
            GameMulti &operator=(GameMulti &&other) = delete;

            ///
            /// @brief Update the game scene (called each frame)
            /// @param dt Delta time since last frame (in seconds)
            /// @param size Current window size
            /// @details Processes network events, updates interpolation, runs game systems
            ///
            void update(float dt, const eng::WindowSize &size) override;

            ///
            /// @brief Handle input events
            /// @param event Input event (keyboard, mouse, etc.)
            ///
            void event(const eng::Event &event) override;

            ///
            /// @brief Update the local player's visual skin/appearance
            /// @details Updates player sprite based on skin index
            ///
            void updatePlayerSkin();

            ///
            /// @brief Get reference to music playback flag
            /// @return Reference to music flag
            ///
            bool &playMusic() { return m_playMusic; }

            /// @brief Callback invoked when game ends
            std::function<void()> onGameOver;

        private:
            ///
            /// @brief Subscribe to event bus events
            /// @details Registers callbacks for world state updates and entity events
            ///
            void setupEventSubscriptions() const;

            ///
            /// @brief Process pending events from the event bus
            /// @details Polls and handles network events from the event bus
            ///
            void processEventBus();

            ///
            /// @brief Handle world state update from server
            /// @param event World state event containing entity data
            /// @details Updates entity positions, creates/destroys entities as needed
            ///
            void handleWorldStateUpdate(const utl::Event &event);

            ///
            /// @brief Preload commonly used textures
            /// @details Loads textures into cache to avoid runtime loading hitches
            ///
            void preloadCommonTextures();

            ///
            /// @brief Update entity position interpolation
            /// @param dataMap Map of entity ID to interpolation data
            /// @param entityMap Map of entity ID to ECS entity
            /// @param smoothFactor Interpolation smoothing factor (0.0-1.0)
            /// @param dt Delta time for frame-rate independent interpolation
            /// @param registry ECS registry containing entities
            /// @details Smoothly interpolates entity positions between server updates
            ///
            void updateInterpolation(std::unordered_map<uint32_t, InterpolationData> &dataMap,
                                     std::unordered_map<uint32_t, ecs::Entity> &entityMap, float smoothFactor, float dt,
                                     ecs::Registry &registry);

            ///
            /// @brief Update entity position interpolation (const version)
            /// @param dataMap Map of entity ID to interpolation data
            /// @param entityMap Map of entity ID to ECS entity
            /// @param smoothFactor Interpolation smoothing factor (0.0-1.0)
            /// @param dt Delta time for frame-rate independent interpolation
            /// @param registry ECS registry containing entities
            ///
            void updateInterpolation(std::unordered_map<uint32_t, InterpolationData> &dataMap,
                                     std::unordered_map<uint32_t, ecs::Entity> &entityMap, float smoothFactor, float dt,
                                     ecs::Registry &registry) const;

            ecs::Entity m_localPlayerEntity;                           ///< Local player entity
            std::unordered_map<uint32_t, ecs::Entity> m_remotePlayers; ///< Map of session ID to remote player entities

            const std::shared_ptr<eng::IAudio> &m_audio;               ///< Audio interface reference
            const std::shared_ptr<eng::IRenderer> &m_renderer;         ///< Renderer interface reference
            float m_skinIndex;                                         ///< Local player skin index
            std::unique_ptr<PlayerControllerMulti> m_playerController; ///< Player input controller
            std::unique_ptr<HUDSystem> m_hudSystem;                    ///< HUD rendering system
            std::unordered_map<eng::Key, bool> m_keysPressed;          ///< Current key press states
            bool &m_showDebug;                                         ///< Debug mode flag reference
            bool m_playMusic = false;                                  ///< Music playback flag

            uint32_t m_sessionId;                         ///< Local player session ID
            uint32_t m_eventComponentId = 10;             ///< Event bus component ID
            std::unique_ptr<StageManager> m_stageManager; ///< Stage/level manager

            std::unordered_map<uint32_t, ecs::Entity> m_projectileEntities; ///< Map of projectile ID to entity
            std::unordered_map<uint32_t, ecs::Entity> m_enemyEntities;      ///< Map of enemy ID to entity

            std::unordered_map<uint32_t, InterpolationData>
                m_remotePlayerData;                                           ///< Interpolation data for remote players
            std::unordered_map<uint32_t, InterpolationData> m_projectileData; ///< Interpolation data for projectiles
            std::unordered_map<uint32_t, InterpolationData> m_enemyData;      ///< Interpolation data for enemies
            std::unordered_map<uint32_t, uint32_t> m_playerSkinMap; ///< Map of player session ID to skin index
            ///
            /// @struct PendingInput
            /// @brief Input command pending server acknowledgment
            /// @details Used for client-side prediction and reconciliation
            ///
            struct PendingInput
            {
                    uint32_t seqId;                 ///< Sequence ID of input
                    std::vector<uint8_t> inputData; ///< Serialized input data
                    float dt;                       ///< Delta time when input was sent
            };
            std::deque<PendingInput> m_inputHistory; ///< History of unacknowledged inputs
            uint32_t m_lastAckSeqId = 0;             ///< Last acknowledged input sequence ID

            static constexpr float REMOTE_PLAYER_SMOOTH_FACTOR = 0.25f; ///< Interpolation speed for remote players
            static constexpr float PROJECTILE_SMOOTH_FACTOR = 0.5f;     ///< Interpolation speed for projectiles
            static constexpr float ENEMY_SMOOTH_FACTOR = 0.18f;         ///< Interpolation speed for enemies

            bool m_starfieldCreated = false;                    ///< Flag indicating if starfield background was created
            ecs::Entity m_beginSoundEntity{};                   ///< Entity for game start sound
            ecs::Entity m_bossMusicEntity{};                    ///< Entity for boss music
            float m_bossMusicTimer = 0.0f;                      ///< Timer for boss music duration
            bool m_bossMusicStarted = false;                    ///< Flag indicating if boss music has started
            static constexpr float BOSS_MUSIC_DURATION = 40.0f; ///< Duration of boss music in seconds

            std::unordered_set<std::string> m_loadedTextures; ///< Cache of loaded texture paths
            std::unordered_set<std::string> m_loadedFonts;    ///< Cache of loaded font paths

            bool m_firstWorldState = true; ///< Flag indicating if this is the first world state (for player skin init)
    }; // class GameMulti
} // namespace gme
