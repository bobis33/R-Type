#include <cmath>

#include "ECS/Component.hpp"
#include "RTypeClientMulti/Scenes/ServerScene.hpp"
#include "Utils/Common.hpp"

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
        case eng::Key::Dot:
            return '.';
        default:
            return '\0';
    }
}

gme::ServerScene::ServerScene(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
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
        .with<ecs::Text>("title", std::string("SERVER"), 72U)
        .build();
    for (size_t i = 0; i < m_serverOptions.size(); ++i)
    {
        float yPosition = 0.0F;
        if (i < 3)
        {
            yPosition = 200.F + i * 50.F;
        }
        else
        {
            yPosition = 200.F + i * 50.F + 30.F;
        }
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_option_" + std::to_string(i), 100.F, yPosition, 0.F)
            .with<ecs::Color>("color_option_" + std::to_string(i), utl::Config::Color::GRAY_BLUE_SUBTLE.r,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.g, utl::Config::Color::GRAY_BLUE_SUBTLE.b,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("option_" + m_serverOptions[i], m_serverOptions[i], 32U)
            .build();
    }
    m_playerNameValueEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_player_name_value", 580.F, 200.F, 0.F)
            .with<ecs::Color>("color_player_name_value", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("player_name_value", m_playerName, 24U)
            .build();

    m_serverIPValueEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_server_ip_value", 580.F, 250.F, 0.F)
            .with<ecs::Color>("color_server_ip_value", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("server_ip_value", m_serverIP, 24U)
            .build();

    m_serverPortValueEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_server_port_value", 580.F, 300.F, 0.F)
            .with<ecs::Color>("color_server_port_value", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("server_port_value", m_serverPort, 24U)
            .build();

    m_eventComponentId = 5;
    m_eventBus.registerComponent(m_eventComponentId, "ServerConnect");
}

void gme::ServerScene::update(const float dt, const eng::WindowSize & /*size*/)
{
    auto &reg = getRegistry();
    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();

    m_animationTime += dt;

    for (auto &[entity, text] : texts)
    {
        for (size_t i = 0; i < m_serverOptions.size(); ++i)
        {
            if (text.id == "option_" + m_serverOptions[i])
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

void gme::ServerScene::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Escape)
            {
                if (onBackToMenu)
                {
                    onBackToMenu();
                }
            }
            else if (event.key == eng::Key::Up)
            {
                m_playMusic = true;
                m_selectedIndex = (m_selectedIndex == 0) ? m_serverOptions.size() - 1 : m_selectedIndex - 1;
            }
            else if (event.key == eng::Key::Down)
            {
                m_playMusic = true;
                m_selectedIndex = (m_selectedIndex == m_serverOptions.size() - 1) ? 0 : m_selectedIndex + 1;
            }
            else if (event.key == eng::Key::Enter)
            {
                if (m_selectedIndex == 3 && onConnect)
                {
                    connectServer(m_playerName, m_serverIP, m_serverPort);
                    onConnect(m_playerName, m_serverIP, m_serverPort);
                }
                else if (m_selectedIndex == 4 && onBackToMenu)
                {
                    onBackToMenu();
                }
            }
            else if (event.key == eng::Key::Delete)
            {
                if (m_selectedIndex < 3)
                {
                    if (std::string &currentField = getCurrentEditField(); !currentField.empty())
                    {
                        currentField.pop_back();
                        updateValueDisplay();
                    }
                }
            }
            else
            {
                if (m_selectedIndex < 3)
                {
                    if (const char c = keyToChar(event.key); c != '\0')
                    {
                        std::string &currentField = getCurrentEditField();
                        if (c == ' ' && m_selectedIndex != 0)
                        {
                            return;
                        }
                        if (currentField.length() < 20)
                        {
                            currentField += c;
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

std::string &gme::ServerScene::getCurrentEditField()
{
    if (m_selectedIndex == 0)
    {
        return m_playerName;
    }
    if (m_selectedIndex == 1)
    {
        return m_serverIP;
    }
    return m_serverPort;
}

void gme::ServerScene::updateValueDisplay()
{
    auto &reg = getRegistry();

    if (auto *playerNameText = reg.getComponent<ecs::Text>(m_playerNameValueEntity))
    {
        playerNameText->content = m_playerName;
    }

    if (auto *serverIPText = reg.getComponent<ecs::Text>(m_serverIPValueEntity))
    {
        serverIPText->content = m_serverIP;
    }

    if (auto *serverPortText = reg.getComponent<ecs::Text>(m_serverPortValueEntity))
    {
        serverPortText->content = m_serverPort;
    }
}

void gme::ServerScene::connectServer(const std::string &playerName, const std::string &serverIP,
                                     const std::string &serverPort) const
{
    rnp::Serializer serializer;
    serializer.writeString(playerName, 32);
    serializer.writeString(serverIP, 15);
    serializer.writeString(serverPort, 5);
    const auto data = serializer.getData();

    m_eventBus.publish(utl::EventType::REQUEST_CONNECT, data, m_eventComponentId, utl::NETWORK_CLIENT);
}
