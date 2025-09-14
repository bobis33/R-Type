///
/// @file RTypeClient.hpp
/// @brief RType client class declaration
/// @namespace rtp
///

#pragma once

#include "Interfaces/IGameClient.hpp"

namespace rtp
{

    ///
    /// @class RTypeClient
    /// @brief Class for the R-Type game
    /// @namespace rtp
    ///
    class RTypeClient final : public cli::IGameClient
    {
        public:
            RTypeClient() = default;
            ~RTypeClient() override = default;

            RTypeClient(const RTypeClient &) = delete;
            RTypeClient &operator=(const RTypeClient &) = delete;
            RTypeClient(RTypeClient &&) = delete;
            RTypeClient &operator=(RTypeClient &&) = delete;

        private:
    };
} // namespace rtp
