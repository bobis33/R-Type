#include "Engine/Engine.hpp"
#include "ECS/Systems.hpp"

eng::Engine::Engine(const std::function<std::unique_ptr<IAudio>()> &audioFactory,
                    const std::function<std::unique_ptr<INetworkClient>()> &networkFactory,
                    const std::function<std::unique_ptr<IRenderer>()> &rendererFactory)
    : m_audio(audioFactory()), m_networkClient(networkFactory()), m_renderer(rendererFactory()),
      m_clock(std::make_unique<utl::Clock>()), m_registry(std::make_unique<ecs::Registry>())
{
}
