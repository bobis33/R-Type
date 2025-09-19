///
/// @file RTypeClient.hpp
/// @brief RType client class declaration
/// @namespace gme
///

#pragma once

#include <memory>

#include "Interfaces/AGameClient.hpp"

namespace gme
{

    ///
    /// @class RTypeClient
    /// @brief Class for the R-Type game
    /// @namespace gme
    ///
    class RTypeClient final : public AGameClient
    {
        public:
            RTypeClient();
            ~RTypeClient() override = default;

            RTypeClient(const RTypeClient &) = delete;
            RTypeClient &operator=(const RTypeClient &) = delete;
            RTypeClient(RTypeClient &&) = delete;
            RTypeClient &operator=(RTypeClient &&) = delete;

            void update(float deltaTime, int width, int height) override;
            [[nodiscard]] const IScene &getCurrentScene() const override { return *m_currentScene; }

        private:
            std::unique_ptr<IScene> m_currentScene;
    }; // class RTypeClient

} // namespace gme
