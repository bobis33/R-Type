#include <unordered_map>

#include <SFML/Audio.hpp>

#include "SFMLAudio/SFMLAudio.hpp"

namespace eng
{
    struct SFMLAudio::Impl
    {
            std::unordered_map<std::string, std::unique_ptr<sf::Music>> musics;
    };

    SFMLAudio::SFMLAudio() : pImpl(std::make_unique<Impl>()) {}
    SFMLAudio::~SFMLAudio() = default;

    void SFMLAudio::createAudio(const std::string &path, const float volume, const bool loop, const std::string &name)
    {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(path))
        {
            return;
        }

        music->setVolume(volume);
        music->setLooping(loop);
        pImpl->musics[name] = std::move(music);
    }

    void SFMLAudio::playAudio(const std::string &name)
    {
        if (const auto it = pImpl->musics.find(name); it != pImpl->musics.end())
            it->second->play();
    }

    void SFMLAudio::setVolume(const std::string &name, const float volume)
    {
        if (const auto it = pImpl->musics.find(name); it != pImpl->musics.end())
            it->second->setVolume(volume);
    }

    void SFMLAudio::setLoop(const std::string &name, const bool loop)
    {
        if (const auto it = pImpl->musics.find(name); it != pImpl->musics.end())
            it->second->setLooping(loop);
    }

    void SFMLAudio::stopAudio(const std::string &name)
    {
        if (const auto it = pImpl->musics.find(name); it != pImpl->musics.end())
        {
            it->second->stop();
        }
    }

    Status SFMLAudio::isPlaying(const std::string &name)
    {
        if (const auto it = pImpl->musics.find(name); it != pImpl->musics.end())
        {
            switch (it->second->getStatus())
            {
                case sf::Music::Status::Playing:
                    return Status::Playing;
                case sf::Music::Status::Paused:
                    return Status::Paused;
                case sf::Music::Status::Stopped:
                    return Status::Stopped;
                default:
                    return Status::Stopped;
            }
        }
        return Status::Stopped;
    }
} // namespace eng