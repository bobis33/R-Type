///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    struct AppConfig;

    ///
    /// @class AudioSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class AudioSystem final : public eng::ASystem
    {
        public:
            AudioSystem(const std::shared_ptr<eng::IAudio> &audio, const AppConfig& appConfig) : m_audio(audio), m_appConfig(appConfig) {}
            ~AudioSystem() override = default;

            AudioSystem(const AudioSystem &) = delete;
            AudioSystem &operator=(const AudioSystem &) = delete;
            AudioSystem(AudioSystem &&) = delete;
            AudioSystem &operator=(AudioSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, audio] : registry.getAll<ecs::Audio>())
                {
                    float globalVolume = static_cast<float>(m_appConfig.audioVolume) / 100.0f;
                    float effectiveVolume = audio.volume * globalVolume;
                    m_audio->setVolume(audio.id + std::to_string(entity), effectiveVolume);
                    m_audio->setLoop(audio.id + std::to_string(entity), audio.loop);
                    if (audio.play && m_audio->isPlaying(audio.id + std::to_string(entity)) != eng::Status::Playing)
                    {
                        m_audio->playAudio(audio.id + std::to_string(entity));
                    }
                    else if (!audio.play &&
                             m_audio->isPlaying(audio.id + std::to_string(entity)) != eng::Status::Stopped)
                    {
                        m_audio->stopAudio(audio.id + std::to_string(entity));
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IAudio> &m_audio;
            const AppConfig& m_appConfig;
    }; // class AudioSystem

} // namespace cli