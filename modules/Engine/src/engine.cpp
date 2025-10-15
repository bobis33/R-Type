#include "Engine/Engine.hpp"

eng::Engine::Engine(const std::function<std::shared_ptr<IAudio>()> &audioFactory,
                    const std::function<std::shared_ptr<INetworkClient>()> &networkFactory,
                    const std::function<std::shared_ptr<IRenderer>()> &rendererFactory)
    : m_clock(std::make_unique<utl::Clock>()), m_sceneManager(std::make_unique<SceneManager>()),
      m_audio(audioFactory()), m_network(networkFactory()), m_renderer(rendererFactory()) {}

void eng::Engine::render(const WindowSize &windowSize, const Color clearColor) const
{
    const float dt = m_clock->getDeltaSeconds();
    m_clock->restart();
    m_renderer->clearWindow(clearColor);
    m_sceneManager->getCurrentScene()->updateSystems(dt);
    m_sceneManager->getCurrentScene()->update(dt, windowSize);
    m_renderer->displayWindow();
}