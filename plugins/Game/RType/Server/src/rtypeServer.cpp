#include "RTypeServer/RTypeServer.hpp"

void gme::RTypeServer::start()
{
    m_gameState = State::PLAYING;

    // create scenes, with necessary entities
}


void gme::RTypeServer::stop() { m_gameState = State::LOSE; }

void gme::RTypeServer::update(const float deltaTime)
{
    // update game logic based on deltaTime
}