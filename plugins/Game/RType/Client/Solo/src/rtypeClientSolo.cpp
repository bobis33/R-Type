#include "RTypeClientSolo/RTypeClientSolo.hpp"
#include "RTypeClientSolo/Scenes/ConfigSolo.hpp"
#include "RTypeClientSolo/Scenes/GameSolo.hpp"
#include "RTypeClientSolo/Systems/Systems.hpp"

void gme::RTypeClientSolo::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    auto configSoloId = m_engine->getSceneManager()->generateNextId();
    auto configSolo = std::make_unique<ConfigSolo>(configSoloId, m_engine->getRenderer(), m_engine->getAudio());
    configSolo->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume));
    configSolo->addSystem(std::make_unique<ecs::PixelSystem>(m_engine->getRenderer()));
    configSolo->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    configSolo->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    auto gameSoloId = m_engine->getSceneManager()->generateNextId();
    auto gameSolo =
        std::make_unique<GameSolo>(gameSoloId, m_engine->getRenderer(), m_engine->getAudio(), m_skinIndex, showDebug);
    gameSolo->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume));
    gameSolo->addSystem(std::make_unique<ecs::PixelSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<AnimationSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<BeamSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<CollisionSystem>(m_engine->getRenderer(), showDebug));
    gameSolo->addSystem(std::make_unique<EnemySystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ExplosionSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<LoadingAnimationSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<PlayerDirectionSystem>(m_skinIndex));
    gameSolo->addSystem(std::make_unique<ProjectileSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ScrollingSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<WeaponSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<SpawnSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), showDebug));
    gameSolo->addSystem(std::make_unique<HUDSystem>(m_engine->getRenderer(), gameSolo->getRegistry()));
    gameSolo->addSystem(std::make_unique<StarfieldSystem>(m_engine->getRenderer(), gameSolo->getRegistry()));
    m_mainSceneId = configSoloId;

    configSolo->onOptionSelected = [this, gameSoloId, menuSceneId](const std::string &option)
    {
        if (option == "Level easy")
        {
            m_engine->getSceneManager()->switchToScene(gameSoloId);
        }
        else if (option == "Level medium")
        {
            m_engine->getSceneManager()->switchToScene(gameSoloId);
        }
        else if (option == "Go back to menu")
        {
            m_engine->getSceneManager()->switchToScene(menuSceneId);
        }
    };

    m_engine->getSceneManager()->addScene(std::move(configSolo));
    m_engine->getSceneManager()->addScene(std::move(gameSolo));
}

void gme::RTypeClientSolo::update(float deltaTime, unsigned int width, unsigned int height) {}
