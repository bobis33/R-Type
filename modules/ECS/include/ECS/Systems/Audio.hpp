///
/// @file Audio.hpp
/// @brief This file contains the audio system definition
/// @namespace ecs
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IAudio.hpp"

namespace ecs
{

    ///
    /// @class AudioSystem
    /// @brief Class for audio system
    /// @namespace ecs
    ///
    class AudioSystem final : public ASystem
    {
        public:
            explicit AudioSystem(const std::shared_ptr<eng::IAudio> &audio, const float &audioVolume)
                : m_audio(audio), m_audioVolume(audioVolume)
            {
            }
            ~AudioSystem() override = default;

            AudioSystem(const AudioSystem &) = delete;
            AudioSystem &operator=(const AudioSystem &) = delete;
            AudioSystem(AudioSystem &&) = delete;
            AudioSystem &operator=(AudioSystem &&) = delete;

            void update(Registry &registry, float /* dt */) override
            {
                for (auto &[entity, audio] : registry.getAll<Audio>())
                {
                    std::string audioName = audio.id + std::to_string(entity);
                    m_audio->setVolume(audioName, audio.volume * m_audioVolume);
                    m_audio->setLoop(audioName, audio.loop);

                    const auto status = m_audio->isPlaying(audioName);
                    if (audio.play)
                    {
                        if (status != eng::Status::Playing)
                        {
                            m_audio->playAudio(audioName);
                            if (!audio.loop)
                            {
                                audio.play = false;
                            }
                        }
                    }
                    else if (audio.loop)
                    {
                        if (status != eng::Status::Stopped)
                        {
                            m_audio->stopAudio(audioName);
                        }
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IAudio> &m_audio;
            const float &m_audioVolume;
    }; // class AudioSystem
} // namespace ecs
