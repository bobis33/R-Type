///
/// @file AGameServer.hpp
/// @brief This file contains the game abstract class
/// @namespace srv
///

#pragma once

#include <string>

#include "Interfaces/IGameServer.hpp"

namespace srv
{

    ///
    /// @class AGameServer
    /// @brief Abstraction for the games
    /// @namespace srv
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

} // namespace srv