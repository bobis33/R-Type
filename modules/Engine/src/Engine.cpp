#include "Engine/Engine.hpp"

eng::Engine::Engine(const std::function<std::unique_ptr<IAudio>()> &audioFactory,
                    const std::function<std::unique_ptr<INetworkClient>()> &networkFactory,
                    const std::function<std::unique_ptr<IRenderer>()> &rendererFactory)
    : m_clock(std::make_unique<utl::Clock>()), m_sceneManager(std::make_unique<SceneManager>()),
      m_audio(audioFactory()), m_network(networkFactory()), m_renderer(rendererFactory())
{
}

void eng::Engine::updateSystems(ecs::Registry &registry, const float dt) const
{
    for (const auto &system : m_systems)
    {
        system->update(registry, dt);
    }
}

void eng::Engine::render(ecs::Registry &registry, const Color clearColor, const float dt) const
{
    m_renderer->clearWindow(clearColor);
    updateSystems(registry, dt);
    m_renderer->displayWindow();
}