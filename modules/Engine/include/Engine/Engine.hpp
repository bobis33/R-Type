///
/// @file Engine.hpp
/// @brief This file contains the Engine class declaration
/// @namespace eng
///

#pragma once

#include <functional>
#include <memory>

#include "ECS/Registry.hpp"
#include "Engine//Systems.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/INetworkClient.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Clock.hpp"

namespace eng
{

    ///
    /// @class Engine
    /// @brief Class for the game engine
    /// @namespace eng
    ///
    class Engine
    {

        public:
            Engine(const std::function<std::unique_ptr<IAudio>()> &audioFactory,
                   const std::function<std::unique_ptr<INetworkClient>()> &networkFactory,
                   const std::function<std::unique_ptr<IRenderer>()> &rendererFactory);
            ~Engine() = default;

            Engine(const Engine &) = delete;
            Engine &operator=(const Engine &) = delete;
            Engine(Engine &&) = delete;
            Engine &operator=(Engine &&) = delete;

            std::unique_ptr<IAudio> &getAudio() { return m_audio; }
            std::unique_ptr<INetworkClient> &getNetworkClient() { return m_networkClient; }
            std::unique_ptr<IRenderer> &getRenderer() { return m_renderer; }
            std::unique_ptr<utl::Clock> &getClock() { return m_clock; }
            std::unique_ptr<ecs::Registry> &getRegistry() { return m_registry; } // to remove

            void addSystem(std::unique_ptr<ISystem> system) { m_systems.emplace_back(std::move(system)); }
            void updateSystems(const float dt) const
            {
                for (auto &system : m_systems)
                {
                    system->update(*m_registry, dt);
                }
            }

        private:
            std::unique_ptr<IAudio> m_audio;
            std::unique_ptr<INetworkClient> m_networkClient;
            std::unique_ptr<IRenderer> m_renderer;

            std::unique_ptr<utl::Clock> m_clock;
            std::unique_ptr<ecs::Registry> m_registry; // to remove, maybe one registry per scene
            std::vector<std::unique_ptr<ISystem>> m_systems;
    }; // class Engine

} // namespace eng