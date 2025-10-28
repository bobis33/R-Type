#include <cmath>

#include "ECS/Component.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "RTypeClientMulti/Scenes/JoinRoom.hpp"
#include "Utils/Common.hpp"
#include "Utils/Logger.hpp"

gme::JoinRoomScene::JoinRoomScene(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
    : AScene(assignedId)
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

    registry.createEntity()
        .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
        .with<ecs::Color>("color_title", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g,
                          utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
        .with<ecs::Text>("title", std::string("JOIN ROOM"), 72U)
        .build();

    m_noRoomsEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_no_rooms", 100.F, 200.F, 0.F)
            .with<ecs::Color>("color_no_rooms", utl::Config::Color::INFO_TEXT_COLOR.r,
                              utl::Config::Color::INFO_TEXT_COLOR.g, utl::Config::Color::INFO_TEXT_COLOR.b,
                              utl::Config::Color::INFO_TEXT_COLOR.a)
            .with<ecs::Text>("no_rooms_text", std::string("No rooms available"), 32U)
            .build();

    registry.createEntity()
        .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_refresh", 100.F, 400.F, 0.F)
        .with<ecs::Color>("color_refresh", utl::Config::Color::GRAY_BLUE_SUBTLE.r,
                          utl::Config::Color::GRAY_BLUE_SUBTLE.g, utl::Config::Color::GRAY_BLUE_SUBTLE.b,
                          utl::Config::Color::GRAY_BLUE_SUBTLE.a)
        .with<ecs::Text>("refresh_text", std::string("Refresh"), 32U)
        .build();

    registry.createEntity()
        .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_back", 100.F, 450.F, 0.F)
        .with<ecs::Color>("color_back", utl::Config::Color::GRAY_BLUE_SUBTLE.r, utl::Config::Color::GRAY_BLUE_SUBTLE.g,
                          utl::Config::Color::GRAY_BLUE_SUBTLE.b, utl::Config::Color::GRAY_BLUE_SUBTLE.a)
        .with<ecs::Text>("back_text", std::string("Back"), 32U)
        .build();

    m_eventComponentId = 7;
    m_eventBus.registerComponent(m_eventComponentId, "Join_Room_Scene");
    setupEventSubscriptions();
    refreshRoomList();
}

void gme::JoinRoomScene::setupEventSubscriptions() const
{
    m_eventBus.subscribe(m_eventComponentId, utl::EventType::LOBBY_LIST_RESPONSE);
    m_eventBus.subscribe(m_eventComponentId, utl::EventType::LOBBY_JOIN_RESPONSE);
}

void gme::JoinRoomScene::handleLobbyListResponse(const utl::Event &event)
{
    try
    {
        rnp::Serializer serializer(event.data);
        auto [lobbyCount, lobbies] = serializer.deserializeLobbyListResponse();

        utl::Logger::log("JoinRoomScene: Received lobby list with " + std::to_string(lobbyCount) + " lobbies",
                         utl::LogLevel::INFO);

        setRooms(lobbies);

        for (const auto &lobby : lobbies)
        {
            size_t nameLen = 0;
            for (size_t i = 0; i < lobby.lobbyName.size() && lobby.lobbyName[i] != '\0'; ++i)
            {
                nameLen = i + 1;
            }
            std::string lobbyName(lobby.lobbyName.data(), nameLen);
            utl::Logger::log("JoinRoomScene: Found lobby '" + lobbyName + "' (ID: " + std::to_string(lobby.lobbyId) +
                                 ") with " + std::to_string(lobby.currentPlayers) + "/" +
                                 std::to_string(lobby.maxPlayers) + " players",
                             utl::LogLevel::INFO);
        }
    }
    catch (const std::exception &e)
    {
        utl::Logger::log("JoinRoomScene: Failed to handle lobby list response - " + std::string(e.what()),
                         utl::LogLevel::WARNING);
    }
}

void gme::JoinRoomScene::handleLobbyJoinResponse(const utl::Event &event) const
{
    try
    {
        rnp::Serializer deserializer(event.data);
        auto [lobbyId, success, errorCode, lobbyInfo] = deserializer.deserializeLobbyJoinResponse();

        if (success == 1)
        {
            utl::Logger::log("JoinRoomScene: Successfully joined lobby " + std::to_string(lobbyId),
                             utl::LogLevel::WARNING);
            if (onJoin)
            {
                onJoin(static_cast<int>(lobbyId), &lobbyInfo);
            }
        }
        else
        {
            std::string errorMsg;
            switch (static_cast<rnp::ErrorCode>(errorCode))
            {
                case rnp::ErrorCode::LOBBY_NOT_FOUND:
                    errorMsg = "Lobby not found";
                    break;
                case rnp::ErrorCode::LOBBY_FULL:
                    errorMsg = "Lobby is full";
                    break;
                case rnp::ErrorCode::ALREADY_IN_LOBBY:
                    errorMsg = "Already in a lobby";
                    break;
                default:
                    errorMsg = "Failed to join lobby";
                    break;
            }
            utl::Logger::log("JoinRoomScene: " + errorMsg, utl::LogLevel::WARNING);
        }
    }
    catch (const std::exception &e)
    {
        utl::Logger::log("JoinRoomScene: Failed to handle lobby join response - " + std::string(e.what()),
                         utl::LogLevel::WARNING);
    }
}

void gme::JoinRoomScene::update(const float dt, const eng::WindowSize & /*size*/)
{
    auto &reg = getRegistry();
    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();
    auto &audios = reg.getAll<ecs::Audio>();

    m_animationTime += dt;

    for (auto &[entity, text] : texts)
    {
        if (text.id == "refresh_text")
        {
            auto &color = colors.at(entity);
            if (m_selectedIndex == m_rooms.size())
            {
                const float glowIntensity = std::sin(m_animationTime * 2.5F);
                color.r = 0U;
                color.g = static_cast<unsigned char>(191U + (glowIntensity * 50));
                color.b = 255U;
            }
            else
            {
                color.r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
                color.g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
                color.b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
            }
        }
        else if (text.id == "back_text")
        {
            auto &color = colors.at(entity);
            if (m_selectedIndex == m_rooms.size() + 1)
            {
                const float glowIntensity = std::sin(m_animationTime * 2.5F);
                color.r = 0U;
                color.g = static_cast<unsigned char>(191U + (glowIntensity * 50));
                color.b = 255U;
            }
            else
            {
                color.r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
                color.g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
                color.b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
            }
        }
    }

    for (size_t i = 0; i < m_roomEntities.size(); ++i)
    {
        if (auto *color = reg.getComponent<ecs::Color>(m_roomEntities[i]))
        {
            if (i == m_selectedIndex)
            {
                float glowIntensity = std::sin(m_animationTime * 2.5F);
                color->r = 0U;
                color->g = static_cast<unsigned char>(191U + (glowIntensity * 50));
                color->b = 255U;
            }
            else
            {
                color->r = utl::Config::Color::TEXT_VALUE_COLOR.r;
                color->g = utl::Config::Color::TEXT_VALUE_COLOR.g;
                color->b = utl::Config::Color::TEXT_VALUE_COLOR.b;
            }
        }
    }
    processEventBus();
}

void gme::JoinRoomScene::event(const eng::Event &event)
{
    const size_t totalOptions = m_rooms.size() + 2;

    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Escape)
            {
                if (onBackToMulti)
                {
                    onBackToMulti();
                }
            }
            else if (event.key == eng::Key::Up)
            {
                m_playMusic = true;
                if (totalOptions > 0)
                {
                    m_selectedIndex = (m_selectedIndex == 0) ? totalOptions - 1 : m_selectedIndex - 1;
                }
            }
            else if (event.key == eng::Key::Down)
            {
                m_playMusic = true;
                if (totalOptions > 0)
                {
                    m_selectedIndex = (m_selectedIndex == totalOptions - 1) ? 0 : m_selectedIndex + 1;
                }
            }
            else if (event.key == eng::Key::Enter)
            {
                if (m_selectedIndex < m_rooms.size())
                {
                    rnp::PacketLobbyJoin joinPacket{};
                    joinPacket.lobbyId = m_rooms[m_selectedIndex].lobbyId;

                    m_eventBus.publish(utl::EventType::LOBBY_JOIN, joinPacket, m_eventComponentId, utl::NETWORK_CLIENT);
                }
                else if (m_selectedIndex == m_rooms.size() && onRefreshRequest)
                {
                    onRefreshRequest();
                    refreshRoomList();
                }
                else if (m_selectedIndex == m_rooms.size() + 1 && onBackToMulti)
                {
                    onBackToMulti();
                }
            }
            break;
        case eng::EventType::KeyReleased:
            break;
        default:
            break;
    }
}

void gme::JoinRoomScene::setRooms(const std::vector<rnp::LobbyInfo> &rooms)
{
    m_rooms = rooms;
    updateRoomDisplay();
}

void gme::JoinRoomScene::refreshRoomList() const
{
    m_eventBus.publish(utl::EventType::LOBBY_LIST_REQUEST, std::vector<std::uint8_t>(), m_eventComponentId,
                       utl::NETWORK_CLIENT);

    if (onRefreshRequest)
    {
        onRefreshRequest();
    }
}

void gme::JoinRoomScene::updateRoomDisplay()
{
    auto &registry = getRegistry();

    utl::Logger::log("JoinRoomScene: Updating room display with " + std::to_string(m_rooms.size()) + " rooms",
                     utl::LogLevel::INFO);

    clearRoomEntities();

    if (auto *noRoomsText = registry.getComponent<ecs::Text>(m_noRoomsEntity))
    {
        noRoomsText->content = m_rooms.empty() ? "No rooms available" : "";
    }

    for (size_t i = 0; i < m_rooms.size(); ++i)
    {
        const rnp::LobbyInfo &lobby = m_rooms[i];

        size_t nameLen = 0;
        for (size_t j = 0; j < lobby.lobbyName.size() && lobby.lobbyName[j] != '\0'; ++j)
        {
            nameLen = j + 1;
        }
        std::string lobbyName(lobby.lobbyName.data(), nameLen);

        std::string roomText =
            lobbyName + " " + std::to_string(lobby.currentPlayers) + "/" + std::to_string(lobby.maxPlayers);

        utl::Logger::log("JoinRoomScene: Creating entity for room: " + roomText, utl::LogLevel::INFO);

        ecs::Entity roomEntity =
            registry.createEntity()
                .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("transform_room_" + std::to_string(i), 100.F, 200.F + i * 40.F, 0.F)
                .with<ecs::Color>("color_room_" + std::to_string(i), utl::Config::Color::TEXT_VALUE_COLOR.r,
                                  utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                                  utl::Config::Color::TEXT_VALUE_COLOR.a)
                .with<ecs::Text>("room_" + std::to_string(i), roomText, 28U)
                .build();

        m_roomEntities.push_back(roomEntity);
    }

    if (m_selectedIndex >= m_rooms.size() + 2)
    {
        m_selectedIndex = 0;
    }
}

void gme::JoinRoomScene::clearRoomEntities()
{
    auto &registry = getRegistry();

    for (const ecs::Entity entity : m_roomEntities)
    {
        if (registry.getComponent<ecs::Text>(entity) != nullptr)
        {
            registry.removeComponent<ecs::Text>(entity);
        }
        if (registry.getComponent<ecs::Color>(entity) != nullptr)
        {
            registry.removeComponent<ecs::Color>(entity);
        }
        if (registry.getComponent<ecs::Transform>(entity) != nullptr)
        {
            registry.removeComponent<ecs::Transform>(entity);
        }
        if (registry.getComponent<ecs::Font>(entity) != nullptr)
        {
            registry.removeComponent<ecs::Font>(entity);
        }
    }

    m_roomEntities.clear();
}

void gme::JoinRoomScene::processEventBus()
{
    for (const std::vector<utl::Event> events = m_eventBus.consumeForTarget(m_eventComponentId);
         const auto &event : events)
    {
        switch (event.type)
        {
            case utl::EventType::LOBBY_LIST_RESPONSE:
                handleLobbyListResponse(event);
                break;
            case utl::EventType::LOBBY_JOIN_RESPONSE:
                handleLobbyJoinResponse(event);
                break;
            default:
                break;
        }
    }
}
