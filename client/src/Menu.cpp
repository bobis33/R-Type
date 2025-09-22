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
        m_engine.getRegistry()->addComponent<ecs::Transform>(titleEntity, "title", 200.f, 50.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(titleEntity, "title", 255, 255, 255, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(titleEntity, "title", "R-Type", 60);

        const auto startEntity = m_engine.getRegistry()->createEntity();
        m_items.push_back({startEntity, "start"});
        m_engine.getRegistry()->addComponent<ecs::Transform>(startEntity, "start", 220.f, 200.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(startEntity, "start", 200, 200, 200, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(startEntity, "start", "Start Game", 30);

        const auto quitEntity = m_engine.getRegistry()->createEntity();
        m_items.push_back({quitEntity, "quit"});
        m_engine.getRegistry()->addComponent<ecs::Transform>(quitEntity, "quit", 220.f, 250.f, 0.f);
        m_engine.getRegistry()->addComponent<ecs::Color>(quitEntity, "quit", 200, 200, 200, 255);
        m_engine.getRegistry()->addComponent<ecs::Text>(quitEntity, "quit", "Quit", 30);

        updateHighlight();

        std::cout << "Menu entities created!" << std::endl;
    }

    void Menu::updateHighlight()
    {
        for (int i = 0; i < static_cast<int>(m_items.size()); i++)
        {
            auto *color = m_engine.getRegistry()->getComponent<ecs::Color>(m_items[i].entity);
            auto *text = m_engine.getRegistry()->getComponent<ecs::Text>(m_items[i].entity);
            if (!color || !text) continue;

            if (i == m_selectedIndex)
            {
                color->r = color->g = color->b = 300;
                text->fontSize = 40;
            } else {
                color->r = color->g = color->b = 200;
                text->fontSize = 30;
            }
        }
    }

    void Menu::handleEvent(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed)
        {
            if (event.key == eng::Key::Up)
            {
                m_selectedIndex = (m_selectedIndex - 1 + m_items.size()) % m_items.size();
                updateHighlight();
            }
            else if (event.key == eng::Key::Down)
            {
                m_selectedIndex = (m_selectedIndex + 1) % m_items.size();
                updateHighlight();
            }
            else if (event.key == eng::Key::Enter)
            {
                if (m_items[m_selectedIndex].id == "start")
                    m_startGame = true;
                else if (m_items[m_selectedIndex].id == "quit")
                    m_exitGame = true;
            }
        }
    }

    void Menu::update(float /*dt*/) 
    {
    }

    void Menu::unload()
    {
        for (auto &item : m_items)
        {
            m_engine.getRegistry()->destroyEntity(item.entity);
        }
        m_items.clear();
        std::cout << "Menu entities destroyed !" << std::endl;
    }
}
