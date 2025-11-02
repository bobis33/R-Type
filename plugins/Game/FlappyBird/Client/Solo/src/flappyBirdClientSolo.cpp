#include "FlappyBirdClientSolo/FlappyBirdClientSolo.hpp"
#include "ECS/Systems/Systems.hpp"
#include "FlappyBirdClientSolo/Scenes/Game.hpp"
#include "FlappyBirdClientSolo/Scenes/ConfigFlappy.hpp"
#include "FlappyBirdClientSolo/Systems/FlappyDebugSystem.hpp"

void gme::FlappyBirdClientSolo::update(float deltaTime, unsigned int width, unsigned int height) {}

void gme::FlappyBirdClientSolo::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    auto configFlappyId = m_engine->getSceneManager()->generateNextId();
    auto configFlappy = std::make_unique<ConfigFlappy>(configFlappyId, m_engine->getRenderer());
    configFlappy->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_appConfig->audioVolume,
                                                               configFlappy->getRegistry(), configFlappy->playMusic()));
    configFlappy->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), showDebug));
    configFlappy->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    configFlappy->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    configFlappy->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), configFlappy->getRegistry()));
    
    auto gameSoloId = m_engine->getSceneManager()->generateNextId();
    auto gameSolo = std::make_unique<Game>(gameSoloId, m_engine->getRenderer(), m_engine->getAudio(), showDebug);
    gameSolo->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_appConfig->audioVolume,
                                                           gameSolo->getRegistry(), gameSolo->playMusic()));
    gameSolo->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), gameSolo->getRegistry()));
    gameSolo->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<FlappyDebugSystem>(m_engine->getRenderer(), showDebug));
    
    m_mainSceneId = configFlappyId;

    configFlappy->onOptionSelected = [this, gameSoloId, menuSceneId](const std::string &option, const std::string &playerName, int skinIndex)
    {
        if (option == "Player Name")
        {
            m_engine->getSceneManager()->switchToScene(menuSceneId);
        }
        else if (option == "Skin")
        {
            auto &sceneManager = m_engine->getSceneManager();
            auto newGame = std::make_unique<Game>(gameSoloId, m_engine->getRenderer(), m_engine->getAudio(), *m_showDebug, skinIndex, playerName);
            newGame->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_appConfig->audioVolume,
                                                                 newGame->getRegistry(), newGame->playMusic()));
            newGame->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), newGame->getRegistry()));
            newGame->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
            newGame->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
            newGame->addSystem(std::make_unique<FlappyDebugSystem>(m_engine->getRenderer(), *m_showDebug));
            sceneManager->addScene(std::move(newGame));
            sceneManager->switchToScene(gameSoloId);
        }
        else if (option == "Start Game")
        {
        }
        else if (option == "Go back to menu")
        {
        }
    };

    m_engine->getSceneManager()->addScene(std::move(configFlappy));
    m_engine->getSceneManager()->addScene(std::move(gameSolo));
}
