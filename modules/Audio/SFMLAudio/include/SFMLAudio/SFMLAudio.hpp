///
/// @file SFMLAudio.hpp
/// @brief SFMLAudio class declaration
/// @namespace eng
///

#pragma once

#include <memory>
#include <string>

#include "Interfaces/IAudio.hpp"

namespace eng
{

    ///
    /// @class SFMLAudio
    /// @brief Class for audio management
    /// @namespace eng
    ///
    class SFMLAudio final : public IAudio
    {
        public:
            SFMLAudio();
            ~SFMLAudio() override;

            SFMLAudio(const SFMLAudio &) = delete;
            SFMLAudio &operator=(const SFMLAudio &) = delete;
            SFMLAudio(SFMLAudio &&) = delete;
            SFMLAudio &operator=(SFMLAudio &&) = delete;

            void createAudio(const std::string &path, float volume, bool loop, const std::string &name) override;
            void playAudio(const std::string &name) override;
            void setVolume(const std::string &name, float volume) override;
            void setLoop(const std::string &name, bool loop) override;
            void stopAudio(const std::string &name) override;
            Status isPlaying(const std::string &name) override;

        private:
            struct Impl;
            std::unique_ptr<Impl> pImpl;
    }; // class SFMLRenderer

} // namespace eng
