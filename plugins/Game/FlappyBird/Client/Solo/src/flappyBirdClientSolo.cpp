#include "FlappyBirdClientSolo/FlappyBirdClientSolo.hpp"
#include "ECS/Systems/Systems.hpp"
#include "FlappyBirdClientSolo/Scenes/Game.hpp"

void gme::FlappyBirdClientSolo::update(float deltaTime, unsigned int width, unsigned int height) {}

void gme::FlappyBirdClientSolo::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    bool f = false;
    auto gameSoloId = m_engine->getSceneManager()->generateNextId();
    auto gameSolo = std::make_unique<Game>(gameSoloId, m_engine->getRenderer(), m_engine->getAudio(), showDebug);
    gameSolo->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), gameSolo->getRegistry()));
    gameSolo->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_appConfig->audioVolume,
                                                             gameSolo->getRegistry(), f));
    gameSolo->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), showDebug));
    m_mainSceneId = gameSoloId;

    m_engine->getSceneManager()->addScene(std::move(gameSolo));
}
