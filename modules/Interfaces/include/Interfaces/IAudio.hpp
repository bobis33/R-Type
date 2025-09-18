///
/// @file IAudio.hpp
/// @brief This file contains the Audio interface
/// @namespace eng
///

#pragma once

#include <string>

namespace eng
{

    ///
    /// @class IAudio
    /// @brief Interface for the audio
    /// @namespace eng
    ///
    class IAudio
    {
        public:
            virtual ~IAudio() = default;

            virtual void createAudio(const std::string &path, float volume, bool loop, const std::string &name) = 0;
            virtual void playAudio(const std::string &name) = 0;

            virtual void setVolume(const std::string &name, float volume) = 0;
            virtual void setLoop(const std::string &name, bool loop) = 0;

        private:
    }; // class IAudio

} // namespace eng