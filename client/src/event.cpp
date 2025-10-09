#include "Client/Client.hpp"

void cli::Client::handleEvents(eng::Event &event)
{
    const auto &scene = m_engine->getSceneManager()->getCurrentScene();

    while (m_engine->getRenderer()->pollEvent(event))
    {
        scene->event(event);

        switch (event.type)
        {
            case eng::EventType::Closed:
                m_engine->setState(eng::State::STOP);
                break;

            case eng::EventType::KeyPressed:
                // if (event.key == eng::Key::Escape)
                //{
                //     m_engine->getRenderer()->closeWindow();
                //     m_engine->setState(eng::State::STOP);
                // }
                // else
                //{
                m_keysPressed[event.key] = true;
                //}
                break;

            case eng::EventType::KeyReleased:
                m_keysPressed[event.key] = false;
                break;

            default:
                break;
        }
    }
}