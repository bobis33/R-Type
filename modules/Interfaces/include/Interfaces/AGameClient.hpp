///
/// @file AGameClient.hpp
/// @brief This file contains the game abstract class
/// @namespace cli
///

#pragma once

#include <string>

#include "Interfaces/IGameClient.hpp"

namespace cli
{

    ///
    /// @class AGameClient
    /// @brief Abstraction for the games
    /// @namespace cli
    ///
    class AGameClient : public IGameClient
    {
        public:
            ~AGameClient() override = default;

            [[nodiscard]] std::string &getName() override { return m_name; }
            void setName(const std::string &newName) override { m_name = newName; }

        private:
            std::string m_name = "default_name";
    }; // class AGameClient

} // namespace cli