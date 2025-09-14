///
/// @file IGameServer.hpp
/// @brief This file contains the Game interface
/// @namespace srv
///

#pragma once

namespace srv
{

    ///
    /// @class IGameServer
    /// @brief Interface for the games
    /// @namespace srv
    ///
    class IGameServer
    {
        public:
            virtual ~IGameServer() = default;

            [[nodiscard]] virtual std::string &getName();
            virtual void setName(const std::string &newName);

        private:
    }; // class IGameServer

} // namespace srv