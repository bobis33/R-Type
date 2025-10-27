///
/// @file Audio.hpp
/// @brief This file contains the audio system definition
/// @namespace ecs
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IAudio.hpp"
#include "Utils/Common.hpp"

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
            explicit AudioSystem(const std::shared_ptr<eng::IAudio> &audio, const float &audioVolume, Registry &registry, bool &playSelectionMusic)
                : m_audio(audio), m_registry(registry), m_audioVolume(audioVolume), m_playSelectionMusic(playSelectionMusic)
            {

        registry.onComponentAdded(
            [&audio, &registry](const Entity e, const std::type_info &type)
            {
                const auto *audioComp = registry.getComponent<Audio>(e);

                if (type == typeid(Audio))
                {
                    if (audioComp)
                    {
                        audio->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                           audioComp->id + std::to_string(e));
                    }
                }
            });
                m_selectionSoundEntity = registry.createEntity()
                                             .with<Audio>("menu_input", utl::Path::Audio::AUDIO_INPUT, 10.F, false, false)
                                             .build();
                m_selectionSoundName = "menu_input" + std::to_string(m_selectionSoundEntity);
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
                if (m_playSelectionMusic) {
                    if (m_selectionSoundName.empty())
                        return;

                    m_audio->stopAudio(m_selectionSoundName);
                    m_audio->playAudio(m_selectionSoundName);
                    m_playSelectionMusic = false;

                }
            }

        private:
            const std::shared_ptr<eng::IAudio> &m_audio;
            Registry &m_registry;
            const float &m_audioVolume;

            Entity m_selectionSoundEntity{};
            std::string m_selectionSoundName;
            bool &m_playSelectionMusic;
    }; // class AudioSystem
} // namespace ecs
