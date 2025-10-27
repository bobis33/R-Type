#include "RTypeClientMulti/RTypeClientMulti.hpp"
#include "RTypeClientMulti/Scenes/ConfigMulti.hpp"
#include "RTypeClientMulti/Scenes/CreateRoom.hpp"
#include "RTypeClientMulti/Scenes/JoinRoom.hpp"
#include "RTypeClientMulti/Scenes/ServerScene.hpp"
#include "RTypeClientMulti/Scenes/WaitingRoom.hpp"
#include "RTypeShared/Systems/Systems.hpp"
#include "Utils/Logger.hpp"

void gme::RTypeClientMulti::update(float deltaTime, unsigned int width, unsigned int height) {}

void gme::RTypeClientMulti::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    auto serverSceneId = m_engine->getSceneManager()->generateNextId();
    auto serverScene = std::make_unique<ServerScene>(serverSceneId, m_engine->getRenderer());
    m_mainSceneId = serverSceneId;
    serverScene->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume, serverScene->getRegistry(), serverScene->playMusic()));
    serverScene->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), serverScene->getRegistry()));
    serverScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto configMultiId = m_engine->getSceneManager()->generateNextId();
    auto configMulti = std::make_unique<ConfigMulti>(configMultiId, m_engine->getRenderer());
    configMulti->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume, configMulti->getRegistry(), configMulti->playMusic()));
    configMulti->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), configMulti->getRegistry()));
    configMulti->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto createRoomId = m_engine->getSceneManager()->generateNextId();
    auto createRoomScene =
        std::make_unique<CreateRoomScene>(createRoomId, m_engine->getRenderer());
    createRoomScene->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume, createRoomScene->getRegistry(), createRoomScene->playMusic()));
    createRoomScene->addSystem(
        std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), createRoomScene->getRegistry()));
    createRoomScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    createRoomScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    createRoomScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto joinRoomId = m_engine->getSceneManager()->generateNextId();
    auto joinRoomScene = std::make_unique<JoinRoomScene>(joinRoomId, m_engine->getRenderer());
    joinRoomScene->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume, joinRoomScene->getRegistry(), joinRoomScene->playMusic()));
    joinRoomScene->addSystem(
        std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), joinRoomScene->getRegistry()));
    joinRoomScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    joinRoomScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    joinRoomScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    JoinRoomScene *joinRoomScenePtr = joinRoomScene.get();
    auto waitingRoomId = m_engine->getSceneManager()->generateNextId();
    auto waitingRoomScene =
        std::make_unique<WaitingRoomScene>(waitingRoomId, m_engine->getRenderer());
    waitingRoomScene->addSystem(
        std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume, waitingRoomScene->getRegistry(), waitingRoomScene->playMusic()));
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
    waitingRoomScene->onGameStart = [this]()
    {
        utl::Logger::log("RTypeClientMulti: Game starting!", utl::LogLevel::INFO);
        // TODO: Transition to game scene
    };

    m_engine->getSceneManager()->addScene(std::move(serverScene));
    m_engine->getSceneManager()->addScene(std::move(configMulti));
    m_engine->getSceneManager()->addScene(std::move(createRoomScene));
    m_engine->getSceneManager()->addScene(std::move(joinRoomScene));
    m_engine->getSceneManager()->addScene(std::move(waitingRoomScene));
}
