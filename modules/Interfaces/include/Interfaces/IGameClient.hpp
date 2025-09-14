///
/// @file IGameClient.hpp
/// @brief This file contains the Game interface
/// @namespace cli
///

#pragma once

namespace cli
{

    ///
    /// @class IGameClient
    /// @brief Interface for the games
    /// @namespace cli
    ///
    class IGameClient
    {
        public:
            virtual ~IGameClient() = default;

            [[nodiscard]] virtual std::string &getName();
            virtual void setName(const std::string &newName);

        private:
    }; // class IGameClient

} // namespace cli