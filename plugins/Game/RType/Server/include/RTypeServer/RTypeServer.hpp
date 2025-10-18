///
/// @file RTypeServer.hpp
/// @brief This file contains the RTypeServer class declaration
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameServer.hpp"

namespace gme
{

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


    private:

    }; // class RTypeServer
} // namespace gme