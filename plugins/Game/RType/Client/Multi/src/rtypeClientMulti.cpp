#include "RTypeClientMulti/RTypeClientMulti.hpp"
#include "RTypeClientMulti/Scenes/ConfigMulti.hpp"
#include "RTypeClientMulti/Scenes/CreateRoom.hpp"
#include "RTypeClientMulti/Scenes/GameMulti.hpp"
#include "RTypeClientMulti/Scenes/JoinRoom.hpp"
#include "RTypeClientMulti/Scenes/ServerScene.hpp"
#include "RTypeClientMulti/Scenes/WaitingRoom.hpp"
#include "RTypeClientMulti/Systems/Animation.hpp"
#include "RTypeClientMulti/Systems/Beam.hpp"
#include "RTypeClientMulti/Systems/LoadingAnimation.hpp"
#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "RTypeClientMulti/Systems/Projectile.hpp"
#include "RTypeClientMulti/Systems/Weapon.hpp"
#include "RTypeShared/Systems/Systems.hpp"
#include "Utils/Logger.hpp"

void gme::RTypeClientMulti::update(float deltaTime, unsigned int width, unsigned int height) {}

void gme::RTypeClientMulti::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    auto serverSceneId = m_engine->getSceneManager()->generateNextId();
    auto serverScene = std::make_unique<ServerScene>(serverSceneId, m_engine->getRenderer());
    m_mainSceneId = serverSceneId;
    serverScene->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume,
                                                              serverScene->getRegistry(), serverScene->playMusic()));
    serverScene->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), serverScene->getRegistry()));
    serverScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto configMultiId = m_engine->getSceneManager()->generateNextId();
    auto configMulti = std::make_unique<ConfigMulti>(configMultiId, m_engine->getRenderer());
    configMulti->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume,
                                                              configMulti->getRegistry(), configMulti->playMusic()));
    configMulti->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), configMulti->getRegistry()));
    configMulti->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto createRoomId = m_engine->getSceneManager()->generateNextId();
    auto createRoomScene = std::make_unique<CreateRoomScene>(createRoomId, m_engine->getRenderer());
    createRoomScene->addSystem(std::make_unique<ecs::AudioSystem>(
        m_engine->getAudio(), m_audioVolume, createRoomScene->getRegistry(), createRoomScene->playMusic()));
    createRoomScene->addSystem(
        std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), createRoomScene->getRegistry()));
    createRoomScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    createRoomScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    createRoomScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto joinRoomId = m_engine->getSceneManager()->generateNextId();
    auto joinRoomScene = std::make_unique<JoinRoomScene>(joinRoomId, m_engine->getRenderer());
    joinRoomScene->addSystem(std::make_unique<ecs::AudioSystem>(
        m_engine->getAudio(), m_audioVolume, joinRoomScene->getRegistry(), joinRoomScene->playMusic()));
    joinRoomScene->addSystem(
        std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), joinRoomScene->getRegistry()));
    joinRoomScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    joinRoomScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    joinRoomScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    JoinRoomScene *joinRoomScenePtr = joinRoomScene.get();
    auto waitingRoomId = m_engine->getSceneManager()->generateNextId();
    auto waitingRoomScene = std::make_unique<WaitingRoomScene>(waitingRoomId, m_engine->getRenderer());
    waitingRoomScene->addSystem(std::make_unique<ecs::AudioSystem>(
        m_engine->getAudio(), m_audioVolume, waitingRoomScene->getRegistry(), waitingRoomScene->playMusic()));
    waitingRoomScene->addSystem(
        std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), waitingRoomScene->getRegistry()));
    waitingRoomScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    waitingRoomScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    waitingRoomScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    WaitingRoomScene *waitingRoomScenePtr = waitingRoomScene.get();

    serverScene->onConnect =
        [this, configMultiId](const std::string &playerName, const std::string &serverIP, const std::string &serverPort)
    { m_engine->getSceneManager()->switchToScene(configMultiId); };
    serverScene->onBackToMenu = [this, menuSceneId]() { m_engine->getSceneManager()->switchToScene(menuSceneId); };

    configMulti->onOptionSelected = [this, menuSceneId, createRoomId, joinRoomId](const std::string &option)
    {
        if (option == "Create room")
        {
            m_engine->getSceneManager()->switchToScene(createRoomId);
        }
        else if (option == "Join room")
        {
            m_engine->getSceneManager()->switchToScene(joinRoomId);
        }
        else if (option == "Go back to menu")
        {
            m_engine->getSceneManager()->switchToScene(menuSceneId);
        }
    };

    createRoomScene->onCreate = [this, configMultiId](const std::string &roomName, int maxPlayers)
    {
        // Room creation is handled through event bus
        // Response will be processed in CreateRoomScene::processEventBus()
        // We stay on the create room scene until we get a response
    };
    createRoomScene->onRoomCreated =
        [this, waitingRoomId, waitingRoomScenePtr](int lobbyId, const rnp::LobbyInfo *lobbyInfo)
    {
        utl::Logger::log("RTypeClientMulti: Room created with ID " + std::to_string(lobbyId), utl::LogLevel::INFO);
        // Transition to waiting room after successful room creation
        waitingRoomScenePtr->setLobbyId(static_cast<std::uint32_t>(lobbyId));
        waitingRoomScenePtr->setIsHost(true); // Creator is the host
        if (lobbyInfo)
        {
            waitingRoomScenePtr->setLobbyInfo(*lobbyInfo);
        }
        m_engine->getSceneManager()->switchToScene(waitingRoomId);
    };
    createRoomScene->onBackToMulti = [this, configMultiId]()
    { m_engine->getSceneManager()->switchToScene(configMultiId); };

    joinRoomScene->onJoin = [this, waitingRoomId, waitingRoomScenePtr](int roomId, const rnp::LobbyInfo *lobbyInfo)
    {
        utl::Logger::log("RTypeClientMulti: Joined room with ID " + std::to_string(roomId), utl::LogLevel::INFO);
        // Transition to waiting room after successful join
        waitingRoomScenePtr->setLobbyId(static_cast<std::uint32_t>(roomId));
        waitingRoomScenePtr->setIsHost(false); // Joiner is not host
        if (lobbyInfo)
        {
            waitingRoomScenePtr->setLobbyInfo(*lobbyInfo);
        }
        m_engine->getSceneManager()->switchToScene(waitingRoomId);
    };
    joinRoomScene->onBackToMulti = [this, configMultiId]()
    { m_engine->getSceneManager()->switchToScene(configMultiId); };
    joinRoomScene->onRefreshRequest = [joinRoomScenePtr]()
    {
        // Room list refresh is handled through event bus
        // JoinRoomScene::refreshRoomList() publishes LOBBY_LIST_REQUEST
        // Response is processed in JoinRoomScene::handleLobbyListResponse()
    };

    waitingRoomScene->onLeaveLobby = [this, configMultiId]()
    { m_engine->getSceneManager()->switchToScene(configMultiId); };
    waitingRoomScene->onGameStart = [this, waitingRoomScenePtr]()
    {
        utl::Logger::log("RTypeClientMulti: Game starting!", utl::LogLevel::INFO);

        // Get sessionId from network client
        uint32_t sessionId = 0;
        if (m_engine && m_engine->getNetwork())
        {
            sessionId = m_engine->getNetwork()->getSessionId();
            utl::Logger::log("RTypeClientMulti: Using sessionId " + std::to_string(sessionId), utl::LogLevel::INFO);
        }

        // Create and setup GameMulti scene
        // Reuse Solo systems
        // Note: You'll need to include these systems from Solo
        // gameMulti->addSystem(std::make_unique<AnimationSystem>(m_engine->getRenderer()));
        // gameMulti->addSystem(std::make_unique<BeamSystem>(m_engine->getRenderer()));
        // gameMulti->addSystem(std::make_unique<CollisionSystem>(m_engine->getRenderer(), m_showDebug));
        // gameMulti->addSystem(std::make_unique<EnemySystem>(m_engine->getRenderer()));
        // gameMulti->addSystem(std::make_unique<ExplosionSystem>(m_engine->getRenderer()));
        // gameMulti->addSystem(std::make_unique<LoadingAnimationSystem>(m_engine->getRenderer()));
        // gameMulti->addSystem(std::make_unique<PlayerDirectionSystem>(m_skinIndex));
        // gameMulti->addSystem(std::make_unique<ProjectileSystem>(m_engine->getRenderer()));

        auto gameMultiId = m_engine->getSceneManager()->generateNextId();
        auto gameMulti =
            std::make_unique<GameMulti>(gameMultiId, m_engine->getRenderer(), m_engine->getAudio(), m_skinIndex,
                                        m_showDebug, waitingRoomScenePtr->getLobbyId(), sessionId);
        gameMulti->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume,
                                                                gameMulti->getRegistry(), gameMulti->playMusic()));
        gameMulti->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
        gameMulti->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
        gameMulti->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
        gameMulti->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), gameMulti->getRegistry()));
        gameMulti->addSystem(std::make_unique<gme::AnimationSystem>(m_engine->getRenderer()));
        gameMulti->addSystem(std::make_unique<gme::LoadingAnimationSystem>(m_engine->getRenderer()));
        gameMulti->addSystem(std::make_unique<gme::BeamSystem>(m_engine->getRenderer()));
        gameMulti->addSystem(std::make_unique<gme::ProjectileSystem>(m_engine->getRenderer()));
        gameMulti->addSystem(std::make_unique<gme::WeaponSystem>(m_engine->getRenderer()));

        m_engine->getSceneManager()->addScene(std::move(gameMulti));
        m_engine->getSceneManager()->switchToScene(gameMultiId);
    };

    m_engine->getSceneManager()->addScene(std::move(serverScene));
    m_engine->getSceneManager()->addScene(std::move(configMulti));
    m_engine->getSceneManager()->addScene(std::move(createRoomScene));
    m_engine->getSceneManager()->addScene(std::move(joinRoomScene));
    m_engine->getSceneManager()->addScene(std::move(waitingRoomScene));
}
