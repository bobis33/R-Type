///
/// @file RTypeGameMulti.hpp
/// @brief R-Type Multiplayer Game Plugin
/// @namespace gme
///

#pragma once

#include "RTypeGame/AGamePlugin.hpp"

namespace gme
{
    ///
    /// @class RTypeGameMulti
    /// @brief Multiplayer game implementation for R-Type
    /// @namespace gme
    ///
    class RTypeGameMulti final : public AGamePlugin
    {
        public:
            RTypeGameMulti();
            ~RTypeGameMulti() override = default;

            RTypeGameMulti(const RTypeGameMulti &) = delete;
            RTypeGameMulti &operator=(const RTypeGameMulti &) = delete;
            RTypeGameMulti(RTypeGameMulti &&) = delete;
            RTypeGameMulti &operator=(RTypeGameMulti &&) = delete;

        protected:
            void initializeScenes() override;
    }; // class RTypeGameMulti

} // namespace gme

