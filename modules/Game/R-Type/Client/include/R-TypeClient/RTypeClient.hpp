///
/// @file RTypeClient.hpp
/// @brief RType client class declaration
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameClient.hpp"

namespace gme
{

    ///
    /// @class RTypeClient
    /// @brief Class for the R-Type game
    /// @namespace gme
    ///
    class RTypeClient final : public IGameClient
    {
        public:
            RTypeClient() = default;
            ~RTypeClient() override = default;

            RTypeClient(const RTypeClient &) = delete;
            RTypeClient &operator=(const RTypeClient &) = delete;
            RTypeClient(RTypeClient &&) = delete;
            RTypeClient &operator=(RTypeClient &&) = delete;

        private:
    }; // class RTypeClient
} // namespace gme
