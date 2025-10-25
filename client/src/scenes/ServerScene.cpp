#include <Interfaces/Protocol/Serializer.hpp>
#include <Utils/Event.hpp>
#include <array>
#include <cmath>
#include <cstdint>

#include "Client/Common.hpp"
#include "Client/Scenes/ServerScene.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"

static constexpr eng::Color CYAN_ELECTRIC = {0U, 191U, 255U, 255U};
static constexpr eng::Color GRAY_BLUE_SUBTLE = {160U, 160U, 160U, 255U};
static constexpr eng::Color TEXT_VALUE_COLOR = {200U, 200U, 255U, 255U};
static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

namespace cli
{
    static char keyToChar(eng::Key key, bool shift = false)
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

    ServerScene::ServerScene(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                             const std::shared_ptr<eng::IAudio> &audio)
        : AScene(assignedId), m_audio(audio)
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
        registry.createEntity().with<ecs::Audio>("id_audio", Path::Audio::AUDIO_TITLE, 5.F, true, true).build();
        m_titleEntity =
            registry.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
                .with<ecs::Color>("color_title", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, CYAN_ELECTRIC.a)
                .with<ecs::Text>("title", std::string("SERVER"), 72U)
                .build();
        for (size_t i = 0; i < m_serverOptions.size(); ++i)
        {
            float yPosition;
            if (i < 3)
            {
                yPosition = 200.F + i * 50.F;
            }
            else
            {
                yPosition = 200.F + i * 50.F + 30.F;
            }
            registry.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("transform_option_" + std::to_string(i), 100.F, yPosition, 0.F)
                .with<ecs::Color>("color_option_" + std::to_string(i), GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g,
                                  GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
                .with<ecs::Text>("option_" + m_serverOptions[i], m_serverOptions[i], 32U)
                .build();
        }
        m_playerNameValueEntity = registry.createEntity()
                                      .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                                      .with<ecs::Transform>("transform_player_name_value", 580.F, 200.F, 0.F)
                                      .with<ecs::Color>("color_player_name_value", TEXT_VALUE_COLOR.r,
                                                        TEXT_VALUE_COLOR.g, TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
                                      .with<ecs::Text>("player_name_value", m_playerName, 24U)
                                      .build();

        m_serverIPValueEntity = registry.createEntity()
                                    .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                                    .with<ecs::Transform>("transform_server_ip_value", 580.F, 250.F, 0.F)
                                    .with<ecs::Color>("color_server_ip_value", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g,
                                                      TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
                                    .with<ecs::Text>("server_ip_value", m_serverIP, 24U)
                                    .build();

        m_serverPortValueEntity = registry.createEntity()
                                      .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                                      .with<ecs::Transform>("transform_server_port_value", 580.F, 300.F, 0.F)
                                      .with<ecs::Color>("color_server_port_value", TEXT_VALUE_COLOR.r,
                                                        TEXT_VALUE_COLOR.g, TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
                                      .with<ecs::Text>("server_port_value", m_serverPort, 24U)
                                      .build();

        m_eventComponentId = 5;
        m_eventBus.registerComponent(m_eventComponentId, "ServerConnect");
    }

    void cli::ServerScene::update(const float dt, const eng::WindowSize & /*size*/)
    {
        auto &reg = getRegistry();
        auto &colors = reg.getAll<ecs::Color>();
        auto &texts = reg.getAll<ecs::Text>();
        auto &audios = reg.getAll<ecs::Audio>();

        m_animationTime += dt;
        for (auto &audio : audios)
        {
            if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
                m_audio->stopAudio(audio.second.id);
        }

        for (auto &[entity, text] : texts)
        {
            for (size_t i = 0; i < m_serverOptions.size(); ++i)
            {
                if (text.id == "option_" + m_serverOptions[i])
                {
                    auto &color = colors.at(entity);

                    if (i == m_selectedIndex)
                    {
                        float glowIntensity = std::sin(m_animationTime * 2.5f);
                        color.r = 0U;
                        color.g = static_cast<unsigned char>(191U + glowIntensity * 50);
                        color.b = 255U;
                    }
                    else
                    {
                        color.r = GRAY_BLUE_SUBTLE.r;
                        color.g = GRAY_BLUE_SUBTLE.g;
                        color.b = GRAY_BLUE_SUBTLE.b;
                    }
                    break;
                }
            }
        }
    }

    void cli::ServerScene::event(const eng::Event &event)
    {
        switch (event.type)
        {
            case eng::EventType::KeyPressed:
                if (event.key == eng::Key::Escape)
                {
                    if (onBackToMenu)
                        onBackToMenu();
                }
                else if (event.key == eng::Key::Up)
                {
                    m_selectedIndex = (m_selectedIndex == 0) ? m_serverOptions.size() - 1 : m_selectedIndex - 1;
                }
                else if (event.key == eng::Key::Down)
                {
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
                        onBackToMenu();
                }
                else if (event.key == eng::Key::Delete)
                {
                    if (m_selectedIndex < 3)
                    {
                        std::string &currentField = getCurrentEditField();
                        if (!currentField.empty())
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
                        char c = keyToChar(event.key);

                        if (c != '\0')
                        {
                            std::string &currentField = getCurrentEditField();
                            if (c == ' ' && m_selectedIndex != 0)
                                return;
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

    std::string &cli::ServerScene::getCurrentEditField()
    {
        if (m_selectedIndex == 0)
            return m_playerName;
        else if (m_selectedIndex == 1)
            return m_serverIP;
        else
            return m_serverPort;
    }

    void cli::ServerScene::updateValueDisplay()
    {
        auto &reg = getRegistry();

        if (auto *playerNameText = reg.getComponent<ecs::Text>(m_playerNameValueEntity))
            playerNameText->content = m_playerName;

        if (auto *serverIPText = reg.getComponent<ecs::Text>(m_serverIPValueEntity))
            serverIPText->content = m_serverIP;

        if (auto *serverPortText = reg.getComponent<ecs::Text>(m_serverPortValueEntity))
            serverPortText->content = m_serverPort;
    }

    void cli::ServerScene::connectServer(const std::string &playerName, const std::string &serverIP,
                                         const std::string &serverPort)
    {
        rnp::Serializer serializer;
        serializer.writeString(playerName, 32);
        serializer.writeString(serverIP, 15);
        serializer.writeString(serverPort, 5);
        auto data = serializer.getData();

        m_eventBus.publish(utl::EventType::REQUEST_CONNECT, data, m_eventComponentId, utl::NETWORK_CLIENT);
    }
} // namespace cli
