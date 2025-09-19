///
/// @file RTypeServer.hpp
/// @brief RType client class declaration
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameServer.hpp"

namespace gme
{

    ///
    /// @class RTypeServer
    /// @brief Class for the R-Type game
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

        private:
    };
} // namespace gme
