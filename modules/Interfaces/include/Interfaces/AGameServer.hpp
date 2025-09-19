///
/// @file AGameServer.hpp
/// @brief This file contains the game abstract class
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameServer.hpp"

namespace gme
{

    ///
    /// @class AGameServer
    /// @brief Abstraction for the games
    /// @namespace gme
    ///
    class AGameServer : public IGameServer
    {
        public:
            ~AGameServer() override = default;

            [[nodiscard]] std::string &getName() override { return m_name; }
            void setName(const std::string &newName) override { m_name = newName; }

        private:
            std::string m_name = "default_name";
    }; // class AGameServer

} // namespace gme