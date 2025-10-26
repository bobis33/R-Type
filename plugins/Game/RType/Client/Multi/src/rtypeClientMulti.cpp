#include "RTypeClientMulti/RTypeClientMulti.hpp"
#include "RTypeClientMulti/Scenes/ConfigMulti.hpp"
#include "RTypeClientMulti/Scenes/CreateRoom.hpp"
#include "RTypeClientMulti/Scenes/JoinRoom.hpp"
#include "RTypeClientMulti/Scenes/ServerScene.hpp"
#include "RTypeShared/Systems/Systems.hpp"

static std::vector<gme::RoomInfo> g_availableRooms; // TODO: Replace with server-side room management
static int g_nextRoomId = 1;

void gme::RTypeClientMulti::update(float deltaTime, unsigned int width, unsigned int height) {}

void gme::RTypeClientMulti::setupScenes(bool &showDebug, eng::id menuSceneId)
{
    auto serverSceneId = m_engine->getSceneManager()->generateNextId();
    auto serverScene = std::make_unique<ServerScene>(serverSceneId, m_engine->getRenderer(), m_engine->getAudio());
    m_mainSceneId = serverSceneId;
    serverScene->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume));
    serverScene->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), serverScene->getRegistry()));
    serverScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto configMultiId = m_engine->getSceneManager()->generateNextId();
    auto configMulti = std::make_unique<ConfigMulti>(configMultiId, m_engine->getRenderer(), m_engine->getAudio());
    configMulti->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume));
    configMulti->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), configMulti->getRegistry()));
    configMulti->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto createRoomId = m_engine->getSceneManager()->generateNextId();
    auto createRoomScene =
        std::make_unique<CreateRoomScene>(createRoomId, m_engine->getRenderer(), m_engine->getAudio());
    createRoomScene->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume));
    createRoomScene->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), createRoomScene->getRegistry()));
    createRoomScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    createRoomScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    createRoomScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto joinRoomId = m_engine->getSceneManager()->generateNextId();
    auto joinRoomScene = std::make_unique<JoinRoomScene>(joinRoomId, m_engine->getRenderer(), m_engine->getAudio());
    joinRoomScene->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_audioVolume));
    joinRoomScene->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), joinRoomScene->getRegistry()));
    joinRoomScene->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    joinRoomScene->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    joinRoomScene->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    JoinRoomScene *joinRoomScenePtr = joinRoomScene.get();

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
        RoomInfo newRoom;
        newRoom.name = roomName;
        newRoom.currentPlayers = 0;
        newRoom.maxPlayers = maxPlayers;
        newRoom.roomId = g_nextRoomId++;
        g_availableRooms.push_back(newRoom); // TODO: to change

        m_engine->getSceneManager()->switchToScene(configMultiId);
    };
    createRoomScene->onBackToMulti = [this, configMultiId]()
    { m_engine->getSceneManager()->switchToScene(configMultiId); };

    joinRoomScene->onJoin = [](const int roomId)
    {
        for (auto &room : g_availableRooms)
        {
            if (room.roomId == roomId && room.currentPlayers < room.maxPlayers)
            {
                room.currentPlayers++;
                break;
            }
        }
    };
    joinRoomScene->onBackToMulti = [this, configMultiId]()
    { m_engine->getSceneManager()->switchToScene(configMultiId); };
    joinRoomScene->onRefreshRequest = [joinRoomScenePtr]()
    { joinRoomScenePtr->setRooms(g_availableRooms); }; // TODO: to change

    m_engine->getSceneManager()->addScene(std::move(serverScene));
    m_engine->getSceneManager()->addScene(std::move(configMulti));
    m_engine->getSceneManager()->addScene(std::move(createRoomScene));
    m_engine->getSceneManager()->addScene(std::move(joinRoomScene));
}