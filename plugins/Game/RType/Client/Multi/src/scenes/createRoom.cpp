#include <Interfaces/Protocol/Protocol.hpp>
#include <Interfaces/Protocol/Serializer.hpp>
#include <Utils/Event.hpp>
#include <Utils/Logger.hpp>
#include <cmath>
#include <ranges>

#include "ECS/Component.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "RTypeClientMulti/Scenes/CreateRoom.hpp"
#include "Utils/Common.hpp"
#include "Utils/Event.hpp"
#include "Utils/Logger.hpp"

static char keyToChar(const eng::Key key, bool shift = false)
{
    switch (key)
    {
        case eng::Key::Q:
            return shift ? 'A' : 'a';
        case eng::Key::W:
            return shift ? 'Z' : 'z';
        case eng::Key::E:
            return shift ? 'E' : 'e';
        case eng::Key::R:
            return shift ? 'R' : 'r';
        case eng::Key::T:
            return shift ? 'T' : 't';
        case eng::Key::Y:
            return shift ? 'Y' : 'y';
        case eng::Key::U:
            return shift ? 'U' : 'u';
        case eng::Key::I:
            return shift ? 'I' : 'i';
        case eng::Key::O:
            return shift ? 'O' : 'o';
        case eng::Key::P:
            return shift ? 'P' : 'p';

        case eng::Key::A:
            return shift ? 'Q' : 'q';
        case eng::Key::S:
            return shift ? 'S' : 's';
        case eng::Key::D:
            return shift ? 'D' : 'd';
        case eng::Key::F:
            return shift ? 'F' : 'f';
        case eng::Key::G:
            return shift ? 'G' : 'g';
        case eng::Key::H:
            return shift ? 'H' : 'h';
        case eng::Key::J:
            return shift ? 'J' : 'j';
        case eng::Key::K:
            return shift ? 'K' : 'k';
        case eng::Key::M:
            return shift ? 'M' : 'm';
        case eng::Key::L:
            return shift ? 'L' : 'l';

        case eng::Key::Z:
            return shift ? 'W' : 'w';
        case eng::Key::X:
            return shift ? 'X' : 'x';
        case eng::Key::C:
            return shift ? 'C' : 'c';
        case eng::Key::V:
            return shift ? 'V' : 'v';
        case eng::Key::B:
            return shift ? 'B' : 'b';
        case eng::Key::N:
            return shift ? 'N' : 'n';

        case eng::Key::Num0:
            return '0';
        case eng::Key::Num1:
            return '1';
        case eng::Key::Num2:
            return '2';
        case eng::Key::Num3:
            return '3';
        case eng::Key::Num4:
            return '4';
        case eng::Key::Num5:
            return '5';
        case eng::Key::Num6:
            return '6';
        case eng::Key::Num7:
            return '7';
        case eng::Key::Num8:
            return '8';
        case eng::Key::Num9:
            return '9';
        case eng::Key::Space:
            return ' ';
        default:
            return '\0';
    }
}

gme::CreateRoomScene::CreateRoomScene(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
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

    m_titleEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
            .with<ecs::Color>("color_title", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g,
                              utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
            .with<ecs::Text>("title", std::string("CREATE ROOM"), 72U)
            .build();

    for (size_t i = 0; i < m_options.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_option_" + std::to_string(i), 100.F, 200.F + i * 50.F, 0.F)
            .with<ecs::Color>("color_option_" + std::to_string(i), utl::Config::Color::GRAY_BLUE_SUBTLE.r,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.g, utl::Config::Color::GRAY_BLUE_SUBTLE.b,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("option_" + m_options[i], m_options[i], 32U)
            .build();
    }

    m_roomNameValueEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_room_name_value", 580.F, 200.F, 0.F)
            .with<ecs::Color>("color_room_name_value", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("room_name_value", m_roomName, 24U)
            .build();

    m_maxPlayersValueEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_max_players_value", 580.F, 250.F, 0.F)
            .with<ecs::Color>("color_max_players_value", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("max_players_value", std::to_string(m_maxPlayers), 24U)
            .build();
    m_eventComponentId = 6;
    m_eventBus.registerComponent(m_eventComponentId, "Room_Manager");
    m_eventBus.subscribe(m_eventComponentId, utl::EventType::LOBBY_CREATE_RESPONSE);
}

void gme::CreateRoomScene::update(const float dt, const eng::WindowSize & /*size*/)
{
    auto &reg = getRegistry();
    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();

    m_animationTime += dt;

    processEventBus();

    for (auto &[entity, text] : texts)
    {
        for (size_t i = 0; i < m_options.size(); ++i)
        {
            if (text.id == "option_" + m_options[i])
            {
                auto &color = colors.at(entity);

                if (i == m_selectedIndex)
                {
                    const float glowIntensity = std::sin(m_animationTime * 2.5f);
                    color.r = 0U;
                    color.g = static_cast<unsigned char>(191U + glowIntensity * 50);
                    color.b = 255U;
                }
                else
                {
                    color.r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
                    color.g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
                    color.b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
                }
                break;
            }
        }
    }
}

void gme::CreateRoomScene::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Escape)
            {
                if (onBackToMulti)
                    onBackToMulti();
            }
            else if (event.key == eng::Key::Up)
            {
                m_playMusic = true;
                m_selectedIndex = (m_selectedIndex == 0) ? m_options.size() - 1 : m_selectedIndex - 1;
            }
            else if (event.key == eng::Key::Down)
            {
                m_playMusic = true;
                m_selectedIndex = (m_selectedIndex == m_options.size() - 1) ? 0 : m_selectedIndex + 1;
            }
            else if (event.key == eng::Key::Enter)
            {
                if (m_selectedIndex == 2 && onCreate)
                {
                    createRoom();
                    onCreate(m_roomName, m_maxPlayers);
                }
                else if (m_selectedIndex == 3 && onBackToMulti)
                    onBackToMulti();
            }
            else if (event.key == eng::Key::Left || event.key == eng::Key::Right)
            {
                if (m_selectedIndex == 1)
                {
                    if (event.key == eng::Key::Left && m_maxPlayers > 2)
                    {
                        m_maxPlayers--;
                        updateValueDisplay();
                    }
                    else if (event.key == eng::Key::Right && m_maxPlayers < 4)
                    {
                        m_maxPlayers++;
                        updateValueDisplay();
                    }
                }
            }
            else if (event.key == eng::Key::Delete)
            {
                if (m_selectedIndex == 0)
                {
                    if (!m_roomName.empty())
                    {
                        m_roomName.pop_back();
                        updateValueDisplay();
                    }
                }
            }
            else
            {
                if (m_selectedIndex == 0)
                {
                    const char inputChar = keyToChar(event.key);

                    if (inputChar != '\0')
                    {
                        if (m_roomName.length() < 20)
                        {
                            m_roomName += inputChar;
                            updateValueDisplay();
                        }
                    }
                }
            }
            break;
        case eng::EventType::KeyReleased:
            break;
        default:
            break;
    }
}

void gme::CreateRoomScene::updateValueDisplay()
{
    auto &reg = getRegistry();

    if (auto *roomNameText = reg.getComponent<ecs::Text>(m_roomNameValueEntity))
        roomNameText->content = m_roomName;

    if (auto *maxPlayersText = reg.getComponent<ecs::Text>(m_maxPlayersValueEntity))
        maxPlayersText->content = std::to_string(m_maxPlayers);
}

void gme::CreateRoomScene::createRoom() const
{
    utl::Logger::log("CreateRoomScene: Creating room '" + m_roomName + "' with " + std::to_string(m_maxPlayers) +
                         " max players",
                     utl::LogLevel::INFO);

    rnp::PacketLobbyCreate packet{};
    packet.maxPlayers = static_cast<std::uint8_t>(m_maxPlayers);
    packet.gameMode = 0;
    packet.nameLen = static_cast<uint8_t>(m_roomName.size());
    for (size_t i = 0; i < m_roomName.size(); ++i)
    {
        packet.lobbyName[i] = m_roomName[i];
    }

    utl::Logger::log("CreateRoomScene: Publishing LOBBY_CREATE event to component " +
                         std::to_string(utl::NETWORK_CLIENT),
                     utl::LogLevel::INFO);
    m_eventBus.publish(utl::EventType::LOBBY_CREATE, packet, m_eventComponentId, utl::NETWORK_CLIENT);
    utl::Logger::log("CreateRoomScene: LOBBY_CREATE event published successfully", utl::LogLevel::INFO);
}

void gme::CreateRoomScene::processEventBus() const
{
    for (const auto events = m_eventBus.consumeForTarget(m_eventComponentId); const auto &event : events)
    {
        if (event.type == utl::EventType::LOBBY_CREATE_RESPONSE)
        {
            rnp::Serializer serializer(event.data);
            rnp::PacketLobbyCreateResponse packet = serializer.deserializeLobbyCreateResponse();
            if (packet.success != 0u)
            {
                utl::Logger::log("CreateRoomScene: Lobby created successfully with ID " +
                                     std::to_string(packet.lobbyId),
                                 utl::LogLevel::INFO);

                if (onRoomCreated)
                {
                    onRoomCreated(static_cast<int>(packet.lobbyId), nullptr);
                }
            }
            else
            {
                utl::Logger::log("CreateRoomScene: Failed to create lobby, error code " +
                                     std::to_string(packet.errorCode),
                                 utl::LogLevel::WARNING);
            }
        }
    }
}
