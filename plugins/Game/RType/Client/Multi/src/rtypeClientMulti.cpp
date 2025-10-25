#include "RTypeClientMulti/RTypeClientMulti.hpp"
#include "RTypeClientMulti/Scenes/ConfigMulti.hpp"
#include "RTypeClientMulti/Scenes/ServerScene.hpp"
#include "RTypeClientMulti/Systems/Systems.hpp"

void gme::RTypeClientMulti::update(float deltaTime, unsigned int width, unsigned int height) {}

void gme::RTypeClientMulti::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    auto serverSceneId = m_engine->getSceneManager()->generateNextId();
    auto serverScene = std::make_unique<ServerScene>(serverSceneId, m_engine->getRenderer(), m_engine->getAudio());
    m_mainSceneId = serverSceneId;
    std::cout << "RTypeClientMulti: Main scene ID is " << m_mainSceneId << '\n';
    serverScene->addSystem(std::make_unique<AudioSystem>(m_engine->getAudio(), static_cast<float>(m_audioVolume)));
    serverScene->addSystem(std::make_unique<PixelSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<TextSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto configMultiId = m_engine->getSceneManager()->generateNextId();
    auto configMulti = std::make_unique<ConfigMulti>(configMultiId, m_engine->getRenderer(), m_engine->getAudio());
    configMulti->addSystem(std::make_unique<AudioSystem>(m_engine->getAudio(), static_cast<float>(m_audioVolume)));
    configMulti->addSystem(std::make_unique<PixelSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<TextSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<DebugSystem>(m_engine->getRenderer(), m_showDebug));

    serverScene->onConnect =
        [this, configMultiId](const std::string &playerName, const std::string &serverIP, const std::string &serverPort)
    { m_engine->getSceneManager()->switchToScene(configMultiId); };
    serverScene->onBackToMenu = [this, menuSceneId]() { m_engine->getSceneManager()->switchToScene(menuSceneId); };

    configMulti->onOptionSelected = [this, menuSceneId](const std::string &option)
    {
        if (option == "Create room")
        {
            // m_engine->getSceneManager()->switchToScene(createRoomId);
        }
        else if (option == "Join room")
        {
            // m_engine->getSceneManager()->switchToScene(joinRoomId);
        }
        else if (option == "Go back to menu")
        {
            m_engine->getSceneManager()->switchToScene(menuSceneId);
        }
    };

    m_engine->getSceneManager()->addScene(std::move(serverScene));
    m_engine->getSceneManager()->addScene(std::move(configMulti));
}