#include <Interfaces/Protocol/Serializer.hpp>
#include <Utils/Logger.hpp>
#include <cmath>

#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientMulti/Scenes/WaitingRoom.hpp"
#include "Utils/Common.hpp"

static constexpr eng::Color CYAN_ELECTRIC = {0U, 191U, 255U, 255U};
static constexpr eng::Color GRAY_BLUE_SUBTLE = {160U, 160U, 160U, 255U};
static constexpr eng::Color TEXT_VALUE_COLOR = {200U, 200U, 255U, 255U};
static constexpr eng::Color INFO_TEXT_COLOR = {180U, 180U, 180U, 200U};
static constexpr eng::Color GREEN_READY = {0U, 255U, 100U, 255U};

namespace gme
{
    WaitingRoomScene::WaitingRoomScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                                       const std::shared_ptr<eng::IAudio> &audio)
        : eng::AScene(assignedId), m_renderer(renderer), m_audio(audio)
    {
        auto &registry = AScene::getRegistry();

        registry.onComponentAdded(
            [&renderer, &audio, &registry](const ecs::Entity e, const std::type_info &type)
            {
                const auto *audioComp = registry.getComponent<ecs::Audio>(e);
                const auto *colorComp = registry.getComponent<ecs::Color>(e);
                const auto *fontComp = registry.getComponent<ecs::Font>(e);
                const auto *textComp = registry.getComponent<ecs::Text>(e);
                const auto *transform = registry.getComponent<ecs::Transform>(e);

                if (type == typeid(ecs::Text))
                {
                    if (textComp && transform && fontComp)
                    {
                        renderer->createFont(fontComp->id, fontComp->path);
                        renderer->createText(
                            {.font_name = fontComp->id,
                             .color = {.r = colorComp->r, .g = colorComp->g, .b = colorComp->b, .a = colorComp->a},
                             .content = textComp->content,
                             .size = textComp->font_size,
                             .x = transform->x,
                             .y = transform->y,
                             .name = textComp->id});
                    }
                }
                else if (type == typeid(ecs::Audio))
                {
                    if (audioComp)
                    {
                        audio->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                           audioComp->id + std::to_string(e));
                    }
                }
            });

        m_titleEntity =
            registry.createEntity()
                .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
                .with<ecs::Color>("color_title", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, CYAN_ELECTRIC.a)
                .with<ecs::Text>("title", std::string("WAITING ROOM"), 72U)
                .build();

        m_lobbyIdEntity = registry.createEntity()
                              .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                              .with<ecs::Transform>("transform_lobby_id", 100.F, 150.F, 0.F)
                              .with<ecs::Color>("color_lobby_id", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g,
                                                TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
                              .with<ecs::Text>("lobby_id_text", std::string("Lobby ID: ..."), 36U)
                              .build();

        m_playerCountEntity = registry.createEntity()
                                  .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                                  .with<ecs::Transform>("transform_player_count", 100.F, 200.F, 0.F)
                                  .with<ecs::Color>("color_player_count", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g,
                                                    TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
                                  .with<ecs::Text>("player_count_text", std::string("Players: 0/4"), 36U)
                                  .build();

        m_statusEntity = registry.createEntity()
                             .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                             .with<ecs::Transform>("transform_status", 100.F, 250.F, 0.F)
                             .with<ecs::Color>("color_status", INFO_TEXT_COLOR.r, INFO_TEXT_COLOR.g, INFO_TEXT_COLOR.b,
                                               INFO_TEXT_COLOR.a)
                             .with<ecs::Text>("status_text", std::string("Waiting for players..."), 32U)
                             .build();

        m_leaveButtonEntity = registry.createEntity()
                                  .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                                  .with<ecs::Transform>("transform_leave", 100.F, 500.F, 0.F)
                                  .with<ecs::Color>("color_leave", GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g,
                                                    GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
                                  .with<ecs::Text>("leave_text", std::string("> Leave Lobby"), 32U)
                                  .build();

        m_readyButtonEntity = registry.createEntity()
                                  .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                                  .with<ecs::Transform>("transform_ready", 100.F, 450.F, 0.F)
                                  .with<ecs::Color>("color_ready", GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g,
                                                    GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
                                  .with<ecs::Text>("ready_text", std::string("  Ready"), 32U)
                                  .build();

        m_startButtonEntity = registry.createEntity()
                                  .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                                  .with<ecs::Transform>("transform_start", 100.F, 550.F, 0.F)
                                  .with<ecs::Color>("color_start", GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g,
                                                    GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
                                  .with<ecs::Text>("start_text", std::string("> Start Game"), 32U)
                                  .build();

        m_eventComponentId = 8;
        m_eventBus.registerComponent(m_eventComponentId, "Waiting_Room_Scene");
        setupEventSubscriptions();
    }

    void WaitingRoomScene::setupEventSubscriptions()
    {
        m_eventBus.subscribe(m_eventComponentId, utl::EventType::LOBBY_UPDATE);
        m_eventBus.subscribe(m_eventComponentId, utl::EventType::GAME_START);
    }

    void WaitingRoomScene::handleLobbyUpdate(const utl::Event &event)
    {
        try
        {
            rnp::Serializer deserializer(event.data);
            rnp::PacketLobbyUpdate packet = deserializer.deserializeLobbyUpdate();

            utl::Logger::log("WaitingRoomScene: Received lobby update for lobby " +
                                 std::to_string(packet.lobbyInfo.lobbyId) +
                                 " - Players: " + std::to_string(packet.lobbyInfo.currentPlayers) + "/" +
                                 std::to_string(packet.lobbyInfo.maxPlayers),
                             utl::LogLevel::INFO);

            m_currentLobbyInfo = packet.lobbyInfo;
            m_hasLobbyInfo = true;
            updatePlayerDisplay();
        }
        catch (const std::exception &e)
        {
            utl::Logger::log("WaitingRoomScene: Error handling lobby update: " + std::string(e.what()),
                             utl::LogLevel::WARNING);
        }
    }

    void WaitingRoomScene::handleGameStart(const utl::Event &event)
    {
        try
        {
            rnp::Serializer deserializer(event.data);
            rnp::PacketGameStart packet = deserializer.deserializeGameStart();

            utl::Logger::log("WaitingRoomScene: Game starting for lobby " + std::to_string(packet.lobbyId),
                             utl::LogLevel::INFO);

            if (onGameStart)
            {
                onGameStart();
            }
        }
        catch (const std::exception &e)
        {
            utl::Logger::log("WaitingRoomScene: Error handling game start: " + std::string(e.what()),
                             utl::LogLevel::WARNING);
        }
    }

    void WaitingRoomScene::update(float dt, const eng::WindowSize & /*size*/)
    {
        processEventBus();

        m_animationTime += dt;
        auto &registry = AScene::getRegistry();

        // Update button selection visual
        auto *leaveColor = registry.getComponent<ecs::Color>(m_leaveButtonEntity);
        auto *readyColor = registry.getComponent<ecs::Color>(m_readyButtonEntity);
        auto *startColor = registry.getComponent<ecs::Color>(m_startButtonEntity);
        auto *leaveText = registry.getComponent<ecs::Text>(m_leaveButtonEntity);
        auto *readyText = registry.getComponent<ecs::Text>(m_readyButtonEntity);
        auto *startText = registry.getComponent<ecs::Text>(m_startButtonEntity);

        if (leaveColor && readyColor && leaveText && readyText)
        {
            // Reset all buttons
            leaveColor->r = GRAY_BLUE_SUBTLE.r;
            leaveColor->g = GRAY_BLUE_SUBTLE.g;
            leaveColor->b = GRAY_BLUE_SUBTLE.b;
            leaveText->content = "  Leave Lobby";
            
            readyColor->r = GRAY_BLUE_SUBTLE.r;
            readyColor->g = GRAY_BLUE_SUBTLE.g;
            readyColor->b = GRAY_BLUE_SUBTLE.b;
            readyText->content = m_isHost ? "> Ready" : "  Ready";
            
            if (m_isHost && startColor && startText)
            {
                startColor->r = GRAY_BLUE_SUBTLE.r;
                startColor->g = GRAY_BLUE_SUBTLE.g;
                startColor->b = GRAY_BLUE_SUBTLE.b;
                startText->content = "  Start Game";
            }
            
            // Highlight selected button
            if (m_selectedButton == BUTTON_LEAVE)
            {
                leaveColor->r = CYAN_ELECTRIC.r;
                leaveColor->g = CYAN_ELECTRIC.g;
                leaveColor->b = CYAN_ELECTRIC.b;
                leaveText->content = "> Leave Lobby";
            }
            else if (m_selectedButton == BUTTON_READY)
            {
                readyColor->r = CYAN_ELECTRIC.r;
                readyColor->g = CYAN_ELECTRIC.g;
                readyColor->b = CYAN_ELECTRIC.b;
                readyText->content = "> Ready";
            }
            else if (m_selectedButton == BUTTON_START && m_isHost && startColor && startText)
            {
                startColor->r = CYAN_ELECTRIC.r;
                startColor->g = CYAN_ELECTRIC.g;
                startColor->b = CYAN_ELECTRIC.b;
                startText->content = "> Start Game";
            }
        }

        // Pulsing effect on status text
        if (m_hasLobbyInfo)
        {
            auto *statusColor = registry.getComponent<ecs::Color>(m_statusEntity);
            if (statusColor)
            {
                const float pulse = (std::sin(m_animationTime * 2.0F) + 1.0F) / 2.0F;
                statusColor->a = static_cast<std::uint8_t>(150U + pulse * 105.0F);
            }
        }
    }

    void WaitingRoomScene::event(const eng::Event &event)
    {
        switch (event.type)
        {
            case eng::EventType::KeyPressed:
                if (event.key == eng::Key::Up)
                {
                    if (m_isHost)
                    {
                        m_selectedButton = (m_selectedButton - 1 + BUTTON_COUNT) % BUTTON_COUNT;
                    }
                    else
                    {
                        // Host doesn't see START button
                        m_selectedButton = (m_selectedButton - 1 + 2) % 2;
                    }
                }
                else if (event.key == eng::Key::Down)
                {
                    if (m_isHost)
                    {
                        m_selectedButton = (m_selectedButton + 1) % BUTTON_COUNT;
                    }
                    else
                    {
                        m_selectedButton = (m_selectedButton + 1) % 2;
                    }
                }
                else if (event.key == eng::Key::Enter || event.key == eng::Key::Space)
                {
                    if (m_selectedButton == BUTTON_LEAVE)
                    {
                        leaveLobby();
                    }
                    else if (m_selectedButton == BUTTON_READY)
                    {
                        utl::Logger::log("WaitingRoomScene: Player ready", utl::LogLevel::INFO);
                        // TODO: Send ready status to server
                    }
                    else if (m_selectedButton == BUTTON_START && m_isHost)
                    {
                        startGame();
                    }
                }
                else if (event.key == eng::Key::Escape)
                {
                    leaveLobby();
                }
                break;
            default:
                break;
        }
    }

    void WaitingRoomScene::setLobbyId(std::uint32_t lobbyId)
    {
        m_lobbyId = lobbyId;
        auto &registry = AScene::getRegistry();
        auto *lobbyIdText = registry.getComponent<ecs::Text>(m_lobbyIdEntity);
        if (lobbyIdText)
        {
            lobbyIdText->content = "Lobby ID: " + std::to_string(lobbyId);
        }
        utl::Logger::log("WaitingRoomScene: Set lobby ID to " + std::to_string(lobbyId), utl::LogLevel::INFO);
    }

    void WaitingRoomScene::setLobbyInfo(const rnp::LobbyInfo &lobbyInfo)
    {
        m_currentLobbyInfo = lobbyInfo;
        m_hasLobbyInfo = true;
        m_lobbyId = lobbyInfo.lobbyId;
        updatePlayerDisplay();
        utl::Logger::log("WaitingRoomScene: Lobby info set - Players: " + std::to_string(lobbyInfo.currentPlayers) +
                             "/" + std::to_string(lobbyInfo.maxPlayers),
                         utl::LogLevel::INFO);
    }

    void WaitingRoomScene::updatePlayerDisplay()
    {
        if (!m_hasLobbyInfo)
        {
            return;
        }

        auto &registry = AScene::getRegistry();

        // Update lobby ID
        auto *lobbyIdText = registry.getComponent<ecs::Text>(m_lobbyIdEntity);
        if (lobbyIdText)
        {
            lobbyIdText->content = "Lobby ID: " + std::to_string(m_currentLobbyInfo.lobbyId);
        }

        // Update player count
        auto *playerCountText = registry.getComponent<ecs::Text>(m_playerCountEntity);
        if (playerCountText)
        {
            playerCountText->content = "Players: " + std::to_string(m_currentLobbyInfo.currentPlayers) + "/" +
                                       std::to_string(m_currentLobbyInfo.maxPlayers);
        }

        // Update status
        auto *statusText = registry.getComponent<ecs::Text>(m_statusEntity);
        if (statusText)
        {
            if (m_currentLobbyInfo.currentPlayers >= m_currentLobbyInfo.maxPlayers)
            {
                statusText->content = "Lobby is full! Waiting for host to start...";
            }
            else
            {
                statusText->content =
                    "Waiting for " + std::to_string(m_currentLobbyInfo.maxPlayers - m_currentLobbyInfo.currentPlayers) +
                    " more player(s)...";
            }
        }

        // Clear old player entities
        clearPlayerEntities();

        // Create player slot displays
        float startY = 320.0F;
        float spacing = 40.0F;

        for (std::uint8_t i = 0; i < m_currentLobbyInfo.maxPlayers; ++i)
        {
            std::string playerText;
            eng::Color playerColor;

            if (i < m_currentLobbyInfo.currentPlayers)
            {
                playerText = "Player " + std::to_string(i + 1) + " - Connected";
                playerColor = GREEN_READY;
            }
            else
            {
                playerText = "Player " + std::to_string(i + 1) + " - Waiting...";
                playerColor = INFO_TEXT_COLOR;
            }

            ecs::Entity playerEntity =
                registry.createEntity()
                    .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                    .with<ecs::Transform>("transform_player_" + std::to_string(i), 120.F, startY + i * spacing, 0.F)
                    .with<ecs::Color>("color_player_" + std::to_string(i), playerColor.r, playerColor.g, playerColor.b,
                                      playerColor.a)
                    .with<ecs::Text>("player_" + std::to_string(i), playerText, 28U)
                    .build();

            m_playerEntities.push_back(playerEntity);
        }

        utl::Logger::log("WaitingRoomScene: Updated player display - " +
                             std::to_string(m_currentLobbyInfo.currentPlayers) + "/" +
                             std::to_string(m_currentLobbyInfo.maxPlayers) + " players",
                         utl::LogLevel::INFO);
    }

    void WaitingRoomScene::clearPlayerEntities()
    {
        auto &registry = AScene::getRegistry();
        for (const auto &entity : m_playerEntities)
        {
            if (auto *text = registry.getComponent<ecs::Text>(entity))
                registry.removeComponent<ecs::Text>(entity);
            if (auto *color = registry.getComponent<ecs::Color>(entity))
                registry.removeComponent<ecs::Color>(entity);
            if (auto *transform = registry.getComponent<ecs::Transform>(entity))
                registry.removeComponent<ecs::Transform>(entity);
            if (auto *font = registry.getComponent<ecs::Font>(entity))
                registry.removeComponent<ecs::Font>(entity);
        }
        m_playerEntities.clear();
    }

void WaitingRoomScene::leaveLobby()
{
    utl::Logger::log("WaitingRoomScene: Leaving lobby " + std::to_string(m_lobbyId), utl::LogLevel::INFO);

    // Publish lobby leave event to network client
    m_eventBus.publish(utl::EventType::LOBBY_LEAVE, std::vector<std::uint8_t>(), m_eventComponentId,
                       utl::NETWORK_CLIENT);

    // Transition back to multi config
    if (onLeaveLobby)
    {
        onLeaveLobby();
    }
}

void WaitingRoomScene::startGame()
{
    utl::Logger::log("WaitingRoomScene: Starting game for lobby " + std::to_string(m_lobbyId), utl::LogLevel::INFO);

    if (!m_isHost)
    {
        utl::Logger::log("WaitingRoomScene: Only host can start game", utl::LogLevel::WARNING);
        return;
    }

    // Send the START_GAME_REQUEST packet through the event bus
    std::vector<std::uint8_t> packetData;
    packetData.resize(sizeof(std::uint32_t));
    std::memcpy(packetData.data(), &m_lobbyId, sizeof(std::uint32_t));
    
    // Send through LOBBY_CREATE event which will be handled by AsioClient
    // AsioClient needs to check the data and send START_GAME_REQUEST instead
    m_eventBus.publish(utl::EventType::LOBBY_CREATE, packetData, m_eventComponentId, utl::NETWORK_CLIENT);
}

void WaitingRoomScene::processEventBus()
{
        std::vector<utl::Event> events = m_eventBus.consumeForTarget(m_eventComponentId);
        for (const auto &event : events)
        {
            switch (event.type)
            {
                case utl::EventType::LOBBY_UPDATE:
                    handleLobbyUpdate(event);
                    break;
                case utl::EventType::GAME_START:
                    utl::Logger::log("WaitingRoomScene: Received GAME_START event", utl::LogLevel::INFO);
                    if (onGameStart)
                    {
                        onGameStart();
                    }
                    break;
                default:
                    break;
            }
        }
    }

} // namespace gme
