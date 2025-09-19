///
/// @file IGameClient.hpp
/// @brief This file contains the Game interface
/// @namespace gme
///

#pragma once

#include <string>

namespace gme
{

    ///
    /// @class IGameClient
    /// @brief Interface for the games
    /// @namespace gme
    ///
    class IGameClient
    {
        public:
            virtual ~IGameClient() = default;

            [[nodiscard]] virtual std::string &getName();
            virtual void setName(const std::string &newName);

        private:
    }; // class IGameClient

} // namespace gme