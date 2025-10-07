///
/// @file Engine.hpp
/// @brief This file contains the Engine class declaration
/// @namespace eng
///

#pragma once

#include <functional>
#include <memory>

#include "ECS/Registry.hpp"
#include "Engine/SceneManager.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/INetworkClient.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Clock.hpp"

namespace eng
{

    enum State : unsigned char
    {
        STOP = 0,
        RUN = 1,
        DEFAULT = 2,
    };

    ///
    /// @class Engine
    /// @brief Class for the game engine
    /// @namespace eng
    ///
    class Engine
    {

        public:
            Engine(const std::function<std::shared_ptr<IAudio>()> &audioFactory,
                   const std::function<std::shared_ptr<INetworkClient>()> &networkFactory,
                   const std::function<std::shared_ptr<IRenderer>()> &rendererFactory);
            ~Engine() = default;

            Engine(const Engine &) = delete;
            Engine &operator=(const Engine &) = delete;
            Engine(Engine &&) = delete;
            Engine &operator=(Engine &&) = delete;

            std::shared_ptr<IAudio> &getAudio() { return m_audio; }
            std::shared_ptr<INetworkClient> &getNetwork() { return m_network; }
            std::shared_ptr<IRenderer> &getRenderer() { return m_renderer; }
            std::unique_ptr<utl::Clock> &getClock() { return m_clock; }
            std::unique_ptr<SceneManager> &getSceneManager() { return m_sceneManager; }
            State getState() const { return m_state; }

            void addSystem(std::unique_ptr<ISystem> system) { m_systems.emplace_back(std::move(system)); }
            void setState(const State newState) { m_state = newState; }

            void render(ecs::Registry &registry, Color clearColor, float dt) const;
            void stop() const { m_renderer->closeWindow(); }

        private:
            void updateSystems(ecs::Registry &registry, float dt) const;

            State m_state = RUN;
            std::unique_ptr<utl::Clock> m_clock;
            std::unique_ptr<SceneManager> m_sceneManager;
            std::vector<std::unique_ptr<ISystem>> m_systems;
            std::shared_ptr<IAudio> m_audio;
            std::shared_ptr<INetworkClient> m_network;
            std::shared_ptr<IRenderer> m_renderer;
    }; // class Engine

} // namespace eng