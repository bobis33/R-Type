///
/// @file RTypeServer.hpp
/// @brief This file contains the RTypeServer class declaration
/// @namespace gme
///

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

namespace gme
{

    enum class LevelState : uint8_t
    {
        WAITING_FOR_PLAYERS = 0,
        IN_PROGRESS = 1,
        COMPLETED = 2,
        LOOSE = 3,
    };

    ///
    /// @class RTypeServer
    /// @brief Class for the RTypeServer plugin
    /// @namespace gme
    ///
    class RTypeServer final : public IGameServer
    {

        public:
            RTypeServer();
            ~RTypeServer() override = default;

            RTypeServer(const RTypeServer &) = delete;
            RTypeServer &operator=(const RTypeServer &) = delete;
            RTypeServer(RTypeServer &&) = delete;
            RTypeServer &operator=(RTypeServer &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "RType_Server"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::GAME_SERVER; }

            [[nodiscard]] State getState() const override { return m_gameState; }

            void start() override;
            void stop() override;
            void update(float deltaTime) override;

        private:
            void processServerStartEvent(const utl::Event &event);
            void processPlayerInputEvent(const utl::Event &event);
            void updateEntities(float deltaTime);
            void updateSystems(float deltaTime);
            void broadcastWorldState();
            void handlePlayerShooting(std::uint32_t sessionId, float deltaTime);

            utl::EventBus &m_eventBus;
            ecs::Registry m_registry;

            // NEW: Entity management
            std::unique_ptr<EntityManager> m_entityManager;

            // NEW: Game systems
            std::unique_ptr<CollisionSystem> m_collisionSystem;
            std::unique_ptr<EnemyAISystem> m_enemyAISystem;
            std::unique_ptr<EnemySpawnSystem> m_enemySpawnSystem;

            State m_gameState = State::PLAYING;
            LevelState m_levelState = LevelState::WAITING_FOR_PLAYERS;

            // Player input tracking
            std::unordered_map<std::uint32_t, bool> m_playerShooting;
            std::unordered_map<std::uint32_t, float> m_lastShotTime;

            float m_lastBroadcastTime = 0.0f;
            const float BROADCAST_INTERVAL = 1.0f / 60.0f; // 60 Hz
            const float PROJECTILE_COOLDOWN = 0.3f;

    }; // class RTypeServer
} // namespace gme
