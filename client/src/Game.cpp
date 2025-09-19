#include "Client/Game.hpp"
#include "ECS/Component.hpp"
#include <iostream>

namespace cli
{
    Game::Game(eng::Engine &engine)
        : m_engine(engine)
    {
        const auto titleEntity = m_engine.getRegistry()->createEntity();
        m_engine.getRegistry()->addComponent<ecs::Transform>(titleEntity, "title", 10.f, 10.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(titleEntity, "title", 255, 255, 255, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(titleEntity, "title", "RType Client", 50);

        m_fpsEntity = m_engine.getRegistry()->createEntity();
        m_engine.getRegistry()->addComponent<ecs::Transform>(m_fpsEntity, "fps", 10.f, 70.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(m_fpsEntity, "fps", 255, 255, 255, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(m_fpsEntity, "fps", "FPS 0", 20);

        std::cout << "Game entities created !" << std::endl;
    }

    void Game::handleEvent(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed && event.key == eng::Key::Escape)
        {
            std::cout << "Escape pressed in Game" << std::endl;
        }
    }

    void Game::update(float dt)
    {
        if (auto *fpsText = m_engine.getRegistry()->getComponent<ecs::Text>(m_fpsEntity))
        {
            fpsText->content = "FPS " + std::to_string(static_cast<int>(1.f / dt));
        }
    }
}
