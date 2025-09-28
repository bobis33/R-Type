///
/// @file IAudio.hpp
/// @brief This file contains the Audio interface
/// @namespace eng
///

#pragma once

#include <string>

#include "Utils/Interfaces/IPlugin.hpp"

namespace eng
{

    enum class Status
    {
        Stopped,
        Paused,
        Playing
    };

    ///
    /// @class IAudio
    /// @brief Interface for the audio
    /// @namespace eng
    ///
    class IAudio : public utl::IPlugin
    {
        public:
            virtual void createAudio(const std::string &path, float volume, bool loop, const std::string &name) = 0;
            virtual void playAudio(const std::string &name) = 0;

            virtual void setVolume(const std::string &name, float volume) = 0;
            virtual void setLoop(const std::string &name, bool loop) = 0;
            virtual void stopAudio(const std::string &name) = 0;
            virtual Status isPlaying(const std::string &name) = 0;

        private:
    }; // class IAudio

} // namespace eng