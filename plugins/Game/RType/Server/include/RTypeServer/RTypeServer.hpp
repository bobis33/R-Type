///
/// @file RTypeServer.hpp
/// @brief Main game server plugin for R-Type multiplayer
/// @details This file contains the RTypeServer class which implements the core game server
///          logic for R-Type multiplayer. It manages the game state, processes player inputs,
///          updates game entities, handles collision detection, spawns enemies, and broadcasts
///          world state to connected clients. The server is the authoritative source for all
///          gameplay logic to ensure fair and synchronized multiplayer experience.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///</parameter>

#pragma once

#include <memory>
#include <unordered_map>

#include "ECS/Registry.hpp"
#include "Interfaces/IGameServer.hpp"
#include "Utils/EventBus.hpp"

// New includes for entity management
#include "RTypeServer/EntityManager.hpp"
#include "RTypeServer/Systems/CollisionSystem.hpp"
#include "RTypeServer/Systems/EnemyAISystem.hpp"
#include "RTypeServer/Systems/EnemySpawnSystem.hpp"
#include "RTypeServer/WaveManager.hpp"

namespace gme
{

    ///
    /// @enum LevelState
    /// @brief State enumeration for level progression
    /// @details Tracks the current state of the game level for proper game flow management
    ///
    enum class LevelState : uint8_t
    {
        WAITING_FOR_PLAYERS = 0, ///< Waiting for players to join before starting
        IN_PROGRESS = 1,         ///< Game is actively running
        COMPLETED = 2,           ///< Level successfully completed (all waves cleared)
        LOOSE = 3,               ///< Game over (all players dead)
    };

    ///
    /// @class RTypeServer
    /// @brief Main game server plugin for R-Type multiplayer gameplay
    /// @details This class implements the complete server-side game logic including:
    ///          - Entity management (players, enemies, projectiles, powerups)
    ///          - Collision detection and resolution
    ///          - Enemy AI and behavior
    ///          - Wave-based enemy spawning
    ///          - Player input processing
    ///          - Score tracking
    ///          - Game state synchronization
    ///          - World state broadcasting to clients
    ///          - Game over detection
    ///
    /// The server operates at a fixed tick rate and broadcasts world state to all
    /// connected clients. All gameplay logic is server-authoritative to prevent
    /// cheating and ensure synchronized gameplay across all clients.
    ///
    /// @namespace gme
    ///
    class RTypeServer final : public IGameServer
    {

        public:
            ///
            /// @brief Constructor
            /// @details Initializes the game server, ECS registry, and all game systems
            ///
            RTypeServer();

            ///
            /// @brief Destructor
            ///
            ~RTypeServer() override = default;

            /// @brief Deleted copy constructor (non-copyable)
            RTypeServer(const RTypeServer &) = delete;
            /// @brief Deleted copy assignment operator (non-copyable)
            RTypeServer &operator=(const RTypeServer &) = delete;
            /// @brief Deleted move constructor (non-movable)
            RTypeServer(RTypeServer &&) = delete;
            /// @brief Deleted move assignment operator (non-movable)
            RTypeServer &operator=(RTypeServer &&) = delete;

            ///
            /// @brief Get plugin name
            /// @return Plugin name identifier
            ///
            [[nodiscard]] const std::string getName() const override { return "RType_Server"; }

            ///
            /// @brief Get plugin type
            /// @return Plugin type (GAME_SERVER)
            ///
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::GAME_SERVER; }

            ///
            /// @brief Get current game state
            /// @return Current server state
            ///
            [[nodiscard]] State getState() const override { return m_gameState; }

            ///
            /// @brief Start the game server
            /// @details Initializes systems and begins listening for game start events
            ///
            void start() override;

            ///
            /// @brief Stop the game server
            /// @details Cleans up all entities and resets game state
            ///
            void stop() override;

            ///
            /// @brief Update the game server (called each tick)
            /// @param deltaTime Time elapsed since last update (in seconds)
            /// @details Processes events, updates entities, runs game systems, and broadcasts state
            ///
            void update(float deltaTime) override;

        private:
            ///
            /// @brief Process server start event from network layer
            /// @param event Event containing server configuration
            /// @details Initializes game systems when server is ready
            ///
            void processServerStartEvent(const utl::Event &event);

            ///
            /// @brief Process player input event
            /// @param event Event containing player input data (movement, shooting)
            /// @details Applies player input to corresponding player entity
            ///
            void processPlayerInputEvent(const utl::Event &event);

            ///
            /// @brief Process game start event from lobby system
            /// @param event Event containing lobby and player information
            /// @details Creates player entities and starts wave system
            ///
            void processGameStartEvent(const utl::Event &event);

            ///
            /// @brief Process player disconnect event
            /// @param event Event containing disconnected player's session ID
            /// @details Removes player entity and checks for game over
            ///
            void processPlayerDisconnectEvent(const utl::Event &event);

            ///
            /// @brief Update entity lifetimes and physics
            /// @param deltaTime Time elapsed since last frame
            /// @details Updates projectile lifetimes and entity movement
            ///
            void updateEntities(float deltaTime);

            ///
            /// @brief Update all game systems (AI, collision, spawning)
            /// @param deltaTime Time elapsed since last frame
            /// @details Runs enemy AI, collision detection, and wave spawning systems
            ///
            void updateSystems(float deltaTime);

            ///
            /// @brief Broadcast complete world state to all clients
            /// @details Sends WORLD_STATE packet containing all entity positions and states
            ///
            void broadcastWorldState() const;

            ///
            /// @brief Handle player shooting logic
            /// @param sessionId Player's session ID
            /// @param deltaTime Time elapsed since last frame
            /// @details Creates projectiles when player shoots, respecting cooldown
            ///
            void handlePlayerShooting(std::uint32_t sessionId, float deltaTime);

            ///
            /// @brief Check for game over conditions
            /// @details Checks if all players are dead or all waves are complete
            ///
            void checkGameOver();

            utl::EventBus &m_eventBus; ///< Event bus for inter-system communication
            ecs::Registry m_registry;  ///< ECS registry containing all game entities

            // Entity and system management
            std::unique_ptr<EntityManager> m_entityManager;       ///< Centralized entity lifecycle manager
            std::unique_ptr<CollisionSystem> m_collisionSystem;   ///< Collision detection and resolution system
            std::unique_ptr<EnemyAISystem> m_enemyAISystem;       ///< Enemy behavior and AI system
            std::unique_ptr<EnemySpawnSystem> m_enemySpawnSystem; ///< Enemy spawning system (legacy/unused)
            std::unique_ptr<WaveManager> m_waveManager;           ///< Wave-based enemy spawning manager

            State m_gameState = State::PLAYING;                        ///< Current server state
            LevelState m_levelState = LevelState::WAITING_FOR_PLAYERS; ///< Current level state

            // Player input tracking
            std::unordered_map<std::uint32_t, bool> m_playerShooting; ///< Map of session ID to shooting state
            std::unordered_map<std::uint32_t, float> m_lastShotTime;  ///< Map of session ID to last shot timestamp

            float m_lastBroadcastTime = 0.0f;       ///< Time since last world state broadcast
            const float PROJECTILE_COOLDOWN = 0.3f; ///< Cooldown between player shots (seconds)

    }; // class RTypeServer
} // namespace gme
