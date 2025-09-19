///
/// @file IGameServer.hpp
/// @brief This file contains the Game interface
/// @namespace gme
///

#pragma once

#include <string>

namespace gme
{

    ///
    /// @class IGameServer
    /// @brief Interface for the games
    /// @namespace gme
    ///
    class IGameServer
    {
        public:
            virtual ~IGameServer() = default;

            [[nodiscard]] virtual std::string &getName();
            virtual void setName(const std::string &newName);

        private:
    }; // class IGameServer

} // namespace gme