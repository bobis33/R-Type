///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    ///
    /// @class AudioSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class AudioSystem final : public eng::ASystem
    {
        public:
            explicit AudioSystem(eng::IAudio &audio) : m_audio(audio) {}
            ~AudioSystem() override = default;

            AudioSystem(const AudioSystem &) = delete;
            AudioSystem &operator=(const AudioSystem &) = delete;
            AudioSystem(AudioSystem &&) = delete;
            AudioSystem &operator=(AudioSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, audio] : registry.getAll<ecs::Audio>())
                {
                    m_audio.setVolume(audio.id + std::to_string(entity), audio.volume);
                    m_audio.setLoop(audio.id + std::to_string(entity), audio.loop);
                    if (audio.play && m_audio.isPlaying(audio.id + std::to_string(entity)) != eng::Status::Playing)
                    {
                        m_audio.playAudio(audio.id + std::to_string(entity));
                    }
                    else if (!audio.play &&
                             m_audio.isPlaying(audio.id + std::to_string(entity)) != eng::Status::Stopped)
                    {
                        m_audio.stopAudio(audio.id + std::to_string(entity));
                    }
                }
            }

        private:
            eng::IAudio &m_audio;
    }; // class AudioSystem

} // namespace cli