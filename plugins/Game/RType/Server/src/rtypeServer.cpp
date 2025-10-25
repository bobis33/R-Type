#include "RTypeServer/RTypeServer.hpp"
#include "ECS/Component.hpp"
#include "Utils/EventBus.hpp"

gme::RTypeServer::RTypeServer() : m_eventBus(utl::EventBus::getInstance()) {}

void gme::RTypeServer::start(/* vector<clientId> clientIds*/)
{
    m_gameState = State::PLAYING;

    // create scenes, with necessary entities
    uint32_t component = 1;
    m_eventBus.registerComponent(component, "RType-Server");
    m_eventBus.subscribe(component, utl::EventType::PLAYER_INPUT_RECEIVED);
    auto events = m_eventBus.consumeForTarget(component);
    for (auto &event : events)
    {
        switch (event.type)
        {
            case utl::EventType::BROADCAST_WORLD_STATE:

                break;
            case utl::EventType::PLAYER_INPUT_RECEIVED:

                break;
            default:
                break;
        }
    }
}

void gme::RTypeServer::stop() { m_gameState = State::LOSE; }

void gme::RTypeServer::update(const float deltaTime)
{
    // update game logic based on deltaTime
}