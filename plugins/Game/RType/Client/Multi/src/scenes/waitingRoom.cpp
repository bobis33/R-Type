#include <cmath>

#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "RTypeClientMulti/Scenes/WaitingRoom.hpp"
#include "Utils/Common.hpp"
#include "Utils/Logger.hpp"

gme::WaitingRoomScene::WaitingRoomScene(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
    : AScene(assignedId), m_renderer(renderer)
{
    auto &registry = AScene::getRegistry();

    registry.onComponentAdded(
        [&renderer, &registry](const ecs::Entity e, const std::type_info &type)
        {
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
        });

    m_titleEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
            .with<ecs::Color>("color_title", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g,
                              utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
            .with<ecs::Text>("title", std::string("WAITING ROOM"), 72U)
            .build();

    m_lobbyIdEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_lobby_id", 100.F, 150.F, 0.F)
            .with<ecs::Color>("color_lobby_id", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("lobby_id_text", std::string("Lobby ID: ..."), 36U)
            .build();

    m_playerCountEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_player_count", 100.F, 200.F, 0.F)
            .with<ecs::Color>("color_player_count", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("player_count_text", std::string("Players: 0/4"), 36U)
            .build();

    m_statusEntity = registry.createEntity()
                         .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                         .with<ecs::Transform>("transform_status", 100.F, 250.F, 0.F)
                         .with<ecs::Color>("color_status", utl::Config::Color::INFO_TEXT_COLOR.r,
                                           utl::Config::Color::INFO_TEXT_COLOR.g, utl::Config::Color::INFO_TEXT_COLOR.b,
                                           utl::Config::Color::INFO_TEXT_COLOR.a)
                         .with<ecs::Text>("status_text", std::string("Waiting for players..."), 32U)
                         .build();

    m_leaveButtonEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_leave", 100.F, 500.F, 0.F)
            .with<ecs::Color>("color_leave", utl::Config::Color::GRAY_BLUE_SUBTLE.r,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.g, utl::Config::Color::GRAY_BLUE_SUBTLE.b,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("leave_text", std::string("> Leave Lobby"), 32U)
            .build();

    m_readyButtonEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_ready", 100.F, 550.F, 0.F)
            .with<ecs::Color>("color_ready", utl::Config::Color::GRAY_BLUE_SUBTLE.r,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.g, utl::Config::Color::GRAY_BLUE_SUBTLE.b,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("ready_text", std::string("  Ready"), 32U)
            .build();

    m_eventComponentId = 8;
    m_eventBus.registerComponent(m_eventComponentId, "Waiting_Room_Scene");
    setupEventSubscriptions();
}

void gme::WaitingRoomScene::setupEventSubscriptions() const
{
    m_eventBus.subscribe(m_eventComponentId, utl::EventType::LOBBY_UPDATE);
    m_eventBus.subscribe(m_eventComponentId, utl::EventType::GAME_START);
}

void gme::WaitingRoomScene::handleLobbyUpdate(const utl::Event &event)
{
    try
    {
        rnp::Serializer deserializer(event.data);
        auto [lobbyInfo] = deserializer.deserializeLobbyUpdate();

        utl::Logger::log("WaitingRoomScene: Received lobby update for lobby " + std::to_string(lobbyInfo.lobbyId) +
                             " - Players: " + std::to_string(lobbyInfo.currentPlayers) + "/" +
                             std::to_string(lobbyInfo.maxPlayers),
                         utl::LogLevel::INFO);

        m_currentLobbyInfo = lobbyInfo;
        m_hasLobbyInfo = true;
        updatePlayerDisplay();
    }
    catch (const std::exception &e)
    {
        utl::Logger::log("WaitingRoomScene: Error handling lobby update: " + std::string(e.what()),
                         utl::LogLevel::WARNING);
    }
}

void gme::WaitingRoomScene::handleGameStart(const utl::Event &event) const
{
    try
    {
        rnp::Serializer deserializer(event.data);
        auto [lobbyId] = deserializer.deserializeGameStart();

        utl::Logger::log("WaitingRoomScene: Game starting for lobby " + std::to_string(lobbyId), utl::LogLevel::INFO);

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

void gme::WaitingRoomScene::update(const float dt, const eng::WindowSize & /*size*/)
{
    processEventBus();

    m_animationTime += dt;
    auto &registry = AScene::getRegistry();

    auto *leaveColor = registry.getComponent<ecs::Color>(m_leaveButtonEntity);
    auto *readyColor = registry.getComponent<ecs::Color>(m_readyButtonEntity);
    auto *leaveText = registry.getComponent<ecs::Text>(m_leaveButtonEntity);
    auto *readyText = registry.getComponent<ecs::Text>(m_readyButtonEntity);

    if ((leaveColor != nullptr) && (readyColor != nullptr) && (leaveText != nullptr) && (readyText != nullptr))
    {
        if (m_selectedButton == BUTTON_LEAVE)
        {
            leaveColor->r = utl::Config::Color::CYAN_ELECTRIC.r;
            leaveColor->g = utl::Config::Color::CYAN_ELECTRIC.g;
            leaveColor->b = utl::Config::Color::CYAN_ELECTRIC.b;
            leaveText->content = "> Leave Lobby";

            readyColor->r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
            readyColor->g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
            readyColor->b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
            readyText->content = "  Ready";
        }
        else if (m_selectedButton == BUTTON_READY)
        {
            readyColor->r = utl::Config::Color::CYAN_ELECTRIC.r;
            readyColor->g = utl::Config::Color::CYAN_ELECTRIC.g;
            readyColor->b = utl::Config::Color::CYAN_ELECTRIC.b;
            readyText->content = "> Ready";

            leaveColor->r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
            leaveColor->g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
            leaveColor->b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
            leaveText->content = "  Leave Lobby";
        }
    }
    if (m_hasLobbyInfo)
    {
        if (auto *statusColor = registry.getComponent<ecs::Color>(m_statusEntity))
        {
            const float pulse = (std::sin(m_animationTime * 2.0F) + 1.0F) / 2.0F;
            statusColor->a = static_cast<std::uint8_t>(150U + pulse * 105.0F);
        }
    }
}

void gme::WaitingRoomScene::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Up)
            {
                m_playMusic = true;
                m_selectedButton = (m_selectedButton - 1 + BUTTON_COUNT) % BUTTON_COUNT;
            }
            else if (event.key == eng::Key::Down)
            {
                m_playMusic = true;
                m_selectedButton = (m_selectedButton + 1) % BUTTON_COUNT;
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

void gme::WaitingRoomScene::setLobbyId(std::uint32_t lobbyId)
{
    m_lobbyId = lobbyId;
    auto &registry = AScene::getRegistry();
    if (auto *lobbyIdText = registry.getComponent<ecs::Text>(m_lobbyIdEntity))
    {
        lobbyIdText->content = "Lobby ID: " + std::to_string(lobbyId);
    }
    utl::Logger::log("WaitingRoomScene: Set lobby ID to " + std::to_string(lobbyId), utl::LogLevel::INFO);
}

void gme::WaitingRoomScene::setLobbyInfo(const rnp::LobbyInfo &lobbyInfo)
{
    m_currentLobbyInfo = lobbyInfo;
    m_hasLobbyInfo = true;
    m_lobbyId = lobbyInfo.lobbyId;
    updatePlayerDisplay();
    utl::Logger::log("WaitingRoomScene: Lobby info set - Players: " + std::to_string(lobbyInfo.currentPlayers) + "/" +
                         std::to_string(lobbyInfo.maxPlayers),
                     utl::LogLevel::INFO);
}

void gme::WaitingRoomScene::updatePlayerDisplay()
{
    if (!m_hasLobbyInfo)
    {
        return;
    }

    auto &registry = AScene::getRegistry();

    if (auto *lobbyIdText = registry.getComponent<ecs::Text>(m_lobbyIdEntity))
    {
        lobbyIdText->content = "Lobby ID: " + std::to_string(m_currentLobbyInfo.lobbyId);
    }

    if (auto *playerCountText = registry.getComponent<ecs::Text>(m_playerCountEntity))
    {
        playerCountText->content = "Players: " + std::to_string(m_currentLobbyInfo.currentPlayers) + "/" +
                                   std::to_string(m_currentLobbyInfo.maxPlayers);
    }

    // Update status
    if (auto *statusText = registry.getComponent<ecs::Text>(m_statusEntity))
    {
        if (m_currentLobbyInfo.currentPlayers >= m_currentLobbyInfo.maxPlayers)
        {
            statusText->content = "Lobby is full! Waiting for host to start...";
        }
        else
        {
            statusText->content = "Waiting for " +
                                  std::to_string(m_currentLobbyInfo.maxPlayers - m_currentLobbyInfo.currentPlayers) +
                                  " more player(s)...";
        }
    }

    clearPlayerEntities();

    for (std::uint8_t i = 0; i < m_currentLobbyInfo.maxPlayers; ++i)
    {
        constexpr float startY = 320.0F;
        constexpr float spacing = 40.0F;
        std::string playerText;
        eng::Color playerColor{};

        if (i < m_currentLobbyInfo.currentPlayers)
        {
            playerText = "Player " + std::to_string(i + 1) + " - Connected";
            playerColor = utl::Config::Color::GREEN_READY;
        }
        else
        {
            playerText = "Player " + std::to_string(i + 1) + " - Waiting...";
            playerColor = utl::Config::Color::INFO_TEXT_COLOR;
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

    utl::Logger::log("WaitingRoomScene: Updated player display - " + std::to_string(m_currentLobbyInfo.currentPlayers) +
                         "/" + std::to_string(m_currentLobbyInfo.maxPlayers) + " players",
                     utl::LogLevel::INFO);
}

void gme::WaitingRoomScene::clearPlayerEntities()
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

void gme::WaitingRoomScene::leaveLobby() const
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

void gme::WaitingRoomScene::processEventBus()
{
    for (const std::vector<utl::Event> events = m_eventBus.consumeForTarget(m_eventComponentId);
         const auto &event : events)
    {
        switch (event.type)
        {
            case utl::EventType::LOBBY_UPDATE:
                handleLobbyUpdate(event);
                break;
            case utl::EventType::GAME_START:
                handleGameStart(event);
                break;
            default:
                break;
        }
    }
}
