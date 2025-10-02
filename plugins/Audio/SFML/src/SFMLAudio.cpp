#include "SFMLAudio/SFMLAudio.hpp"

void eng::SFMLAudio::createAudio(const std::string &path, const float volume, const bool loop, const std::string &name)
{
    auto music = std::make_unique<sf::Music>();
    if (!music->openFromFile(path))
    {
        return;
    }

    music->setVolume(volume);
    music->setLooping(loop);
    musics[name] = std::move(music);
}

void eng::SFMLAudio::playAudio(const std::string &name)
{
    if (const auto it = musics.find(name); it != musics.end())
        it->second->play();
}

void eng::SFMLAudio::setVolume(const std::string &name, const float volume)
{
    if (const auto it = musics.find(name); it != musics.end())
        it->second->setVolume(volume);
}

void eng::SFMLAudio::setLoop(const std::string &name, const bool loop)
{
    if (const auto it = musics.find(name); it != musics.end())
        it->second->setLooping(loop);
}

void eng::SFMLAudio::stopAudio(const std::string &name)
{
    if (const auto it = musics.find(name); it != musics.end())
    {
        it->second->stop();
    }
}

eng::Status eng::SFMLAudio::isPlaying(const std::string &name)
{
    if (const auto it = musics.find(name); it != musics.end())
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
