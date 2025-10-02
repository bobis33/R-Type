///
/// @file SFMLAudio.hpp
/// @brief SFMLAudio class declaration
/// @namespace eng
///

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Audio.hpp>

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
            SFMLAudio() = default;
            ~SFMLAudio() override = default;

            SFMLAudio(const SFMLAudio &) = delete;
            SFMLAudio &operator=(const SFMLAudio &) = delete;
            SFMLAudio(SFMLAudio &&) = delete;
            SFMLAudio &operator=(SFMLAudio &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Audio_SFML"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::AUDIO; }

            void createAudio(const std::string &path, float volume, bool loop, const std::string &name) override;
            void playAudio(const std::string &name) override;
            void setVolume(const std::string &name, float volume) override;
            void setLoop(const std::string &name, bool loop) override;
            void stopAudio(const std::string &name) override;
            Status isPlaying(const std::string &name) override;

        private:
            std::unordered_map<std::string, std::unique_ptr<sf::Music>> musics;

    }; // class SFMLRenderer

} // namespace eng
