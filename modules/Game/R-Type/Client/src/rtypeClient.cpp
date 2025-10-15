#include "R-TypeClient/RTypeClient.hpp"
#include "R-TypeClient/LobbyScene.hpp"

gme::RTypeClient::RTypeClient() : m_currentScene(std::make_unique<LobbyScene>()) { AGameClient::setName("R-Type"); }

void gme::RTypeClient::update(const float deltaTime, const unsigned int width, const unsigned int height) {}
