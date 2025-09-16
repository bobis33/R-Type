///
/// @file RTypeServer.hpp
/// @brief RType client class declaration
/// @namespace rtp
///

#pragma once

#include "Interfaces/IGameServer.hpp"

namespace rtp
{

    ///
    /// @class RTypeServer
    /// @brief Class for the R-Type game
    /// @namespace rtp
    ///
    class RTypeServer final : public srv::IGameServer
    {
        public:
            RTypeServer() = default;
            ~RTypeServer() override = default;

            RTypeServer(const RTypeServer &) = delete;
            RTypeServer &operator=(const RTypeServer &) = delete;
            RTypeServer(RTypeServer &&) = delete;
            RTypeServer &operator=(RTypeServer &&) = delete;

        private:
    };
} // namespace rtp
