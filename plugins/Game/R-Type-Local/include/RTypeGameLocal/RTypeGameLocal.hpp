///
/// @file RTypeGameLocal.hpp
/// @brief R-Type Local/Solo Game Plugin
/// @namespace gme
///

#pragma once

#include "RTypeGame/AGamePlugin.hpp"

namespace gme
{
    ///
    /// @class RTypeGameLocal
    /// @brief Local/Solo game implementation for R-Type
    /// @namespace gme
    ///
    class RTypeGameLocal final : public AGamePlugin
    {
        public:
            RTypeGameLocal();
            ~RTypeGameLocal() override = default;

            RTypeGameLocal(const RTypeGameLocal &) = delete;
            RTypeGameLocal &operator=(const RTypeGameLocal &) = delete;
            RTypeGameLocal(RTypeGameLocal &&) = delete;
            RTypeGameLocal &operator=(RTypeGameLocal &&) = delete;

        protected:
            void initializeScenes() override;
    }; // class RTypeGameLocal

} // namespace gme

