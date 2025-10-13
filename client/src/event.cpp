#include "Client/Client.hpp"
#include "Utils/Logger.hpp"

void cli::Client::handleEvents(eng::Event &event)
{
    while (m_engine->getRenderer()->pollEvent(event))
    {
        // Forward events to game plugin
        if (m_game)
        {
            m_game->handleEvent(event);
        }

        switch (event.type)
        {
            case eng::EventType::Closed:
                m_engine->setState(eng::State::STOP);
                break;

            case eng::EventType::KeyPressed:
                m_keysPressed[event.key] = true;
                break;

            case eng::EventType::KeyReleased:
                m_keysPressed[event.key] = false;
                break;

            default:
                break;
        }
    }
}