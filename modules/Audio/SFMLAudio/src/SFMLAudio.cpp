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

    void SFMLAudio::createAudio(const std::string &path, float volume, bool loop, const std::string &name)
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
        auto it = pImpl->musics.find(name);
        if (it != pImpl->musics.end())
            it->second->play();
    }

    void SFMLAudio::setVolume(const std::string &name, float volume)
    {
        auto it = pImpl->musics.find(name);
        if (it != pImpl->musics.end())
            it->second->setVolume(volume);
    }

    void SFMLAudio::setLoop(const std::string &name, bool loop)
    {
        auto it = pImpl->musics.find(name);
        if (it != pImpl->musics.end())
            it->second->setLooping(loop);
    }
} // namespace eng