///
/// @file RTypeServer.hpp
/// @brief This file contains the RTypeServer class declaration
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameServer.hpp"

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
            RTypeServer() = default;
            ~RTypeServer() override = default;

            RTypeServer(const RTypeServer &) = delete;
            RTypeServer &operator=(const RTypeServer &) = delete;
            RTypeServer(RTypeServer &&) = delete;
            RTypeServer &operator=(RTypeServer &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "RType_Server"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::GAME_SERVER; }

            [[nodiscard]] State getState() const override { return m_gameState; }
            void setContext(const ServerAPI &api) override { m_ctx = api; }

            void start() override;
            void stop() override;
            void update(float deltaTime) override;

        private:

            ServerAPI m_ctx;

            State m_gameState = State::PLAYING;
            LevelState m_levelState = LevelState::WAITING_FOR_PLAYERS;

    }; // class RTypeServer
} // namespace gme