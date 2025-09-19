#include "Client/Menu.hpp"
#include "ECS/Component.hpp"
#include <iostream>

namespace cli
{
    Menu::Menu(eng::Engine &engine)
        : m_engine(engine)
    {
        createMenuEntities();
    }

    void Menu::createMenuEntities()
    {
        const auto titleEntity = m_engine.getRegistry()->createEntity();
        m_entities.push_back(titleEntity);
        m_engine.getRegistry()->addComponent<ecs::Transform>(titleEntity, "title", 200.f, 50.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(titleEntity, "title", 255, 255, 255, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(titleEntity, "title", "R-Type", 60);

        const auto startEntity = m_engine.getRegistry()->createEntity();
        m_entities.push_back(startEntity);
        m_engine.getRegistry()->addComponent<ecs::Transform>(startEntity, "start", 220.f, 200.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(startEntity, "start", 200, 200, 200, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(startEntity, "start", "Press SPACE to Start", 30);

        const auto quitEntity = m_engine.getRegistry()->createEntity();
        m_entities.push_back(quitEntity);
        m_engine.getRegistry()->addComponent<ecs::Transform>(quitEntity, "quit", 220.f, 250.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(quitEntity, "quit", 200, 200, 200, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(quitEntity, "quit", "Press ESC to Quit", 30);

        std::cout << "Menu entities created!" << std::endl;
    }

    void Menu::handleEvent(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed)
        {
            if (event.key == eng::Key::Escape)
                m_exitGame = true;
            else if (event.key == eng::Key::Space)
                m_startGame = true;
        }
    }

    void Menu::update(float /*dt*/) 
    {
    }

    void Menu::unload()
    {
        for (auto e : m_entities)
        {
            m_engine.getRegistry()->destroyEntity(e);
        }
        m_entities.clear();
        std::cout << "Menu entities destroyed !" << std::endl;
    }
}
