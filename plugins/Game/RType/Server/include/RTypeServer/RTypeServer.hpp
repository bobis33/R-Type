///
/// @file RTypeServer.hpp
/// @brief This file contains the RTypeServer class declaration
/// @namespace gme
///

#pragma once

#include <unordered_map>

#include "ECS/Registry.hpp"
#include "Interfaces/IGameServer.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Utils/EventBus.hpp"

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
            void processInputs();
            void updateEntities(float deltaTime);
            void broadcastWorldState();
            void spawnProjectile(std::uint32_t playerId, float x, float y, float vx, float vy);
            void processServerStartEvents();

            utl::EventBus &m_eventBus;
            ecs::Registry m_registry;

            State m_gameState = State::PLAYING;
            LevelState m_levelState = LevelState::WAITING_FOR_PLAYERS;

            std::unordered_map<std::uint32_t, ecs::Entity> m_playerEntities;
            std::unordered_map<std::uint32_t, ecs::Entity> m_projectileEntities;
            std::unordered_map<std::uint32_t, float> m_lastShotTime;
            std::uint32_t m_nextProjectileId = 1000;

            float m_lastBroadcastTime = 0.0f;
            const float BROADCAST_INTERVAL = 1.0f / 144.0f; // 144 Hz - ultra smooth
            const float PROJECTILE_COOLDOWN = 0.3f;
            const float PROJECTILE_SPEED = 800.0f;

    }; // class RTypeServer
} // namespace gme
