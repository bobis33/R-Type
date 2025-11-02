#include "RTypeClientSolo/RTypeClientSolo.hpp"
#include "RTypeClientSolo/Scenes/ConfigSolo.hpp"
#include "RTypeClientSolo/Scenes/GameSolo.hpp"
#include "RTypeClientSolo/Systems/Systems.hpp"
#include "ECS/Systems/LoadingAnimation.hpp"
#include "ECS/Systems/PlayerDirection.hpp"
#include "ECS/Systems/Projectile.hpp"
#include "ECS/Systems/Scrolling.hpp"

void gme::RTypeClientSolo::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    auto configSoloId = m_engine->getSceneManager()->generateNextId();
    auto configSolo = std::make_unique<ConfigSolo>(configSoloId, m_engine->getRenderer());
    configSolo->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_appConfig->audioVolume,
                                                             configSolo->getRegistry(), configSolo->playMusic()));
    configSolo->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), showDebug));
    configSolo->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    configSolo->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    configSolo->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), configSolo->getRegistry()));
    auto gameSoloId = m_engine->getSceneManager()->generateNextId();
    auto gameSolo = std::make_unique<GameSolo>(gameSoloId, m_engine->getRenderer(), m_engine->getAudio(),
                                               m_appConfig->skinIndex, showDebug);
    gameSolo->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_appConfig->audioVolume,
                                                           gameSolo->getRegistry(), gameSolo->playMusic()));
    gameSolo->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::AnimationSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::BeamSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<CollisionSystem>(m_engine->getRenderer(), showDebug));
    gameSolo->addSystem(std::make_unique<EnemySystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::ExplosionSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::LoadingAnimationSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::PlayerDirectionSystem>([skinIndex = m_appConfig->skinIndex](ecs::Registry &, ecs::Entity, ecs::Rect *) {
        return skinIndex * static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT);
    }));
    gameSolo->addSystem(std::make_unique<ecs::ProjectileSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::ScrollingSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<WeaponSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<SpawnSystem>(m_engine->getRenderer()));
    gameSolo->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), showDebug));
    gameSolo->addSystem(std::make_unique<HUDSystem>(m_engine->getRenderer(), gameSolo->getRegistry()));
    gameSolo->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), gameSolo->getRegistry()));
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
