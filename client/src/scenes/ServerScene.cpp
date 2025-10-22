#include "Client/Scenes/ServerScene.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"

static constexpr eng::Color CYAN_ELECTRIC = {0U, 191U, 255U, 255U};
static constexpr eng::Color GRAY_BLUE_SUBTLE = {160U, 160U, 160U, 255U};
static constexpr eng::Color TEXT_VALUE_COLOR = {200U, 200U, 255U, 255U};
static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

cli::ServerScene::ServerScene(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio)
    : m_audio(audio)
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
    m_titleEntity = registry.createEntity()
                        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                        .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
                        .with<ecs::Color>("color_title", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, CYAN_ELECTRIC.a)
                        .with<ecs::Text>("title", std::string("SERVER CONFIG"), 72U)
                        .build();
    for (size_t i = 0; i < m_serverOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_option_" + std::to_string(i), 100.F, 200.F + i * 50.F, 0.F)
            .with<ecs::Color>("color_option_" + std::to_string(i), GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g,
                              GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("option_" + m_serverOptions[i], m_serverOptions[i], 32U)
            .build();
    }
    m_playerNameValueEntity = registry.createEntity()
                                  .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                                  .with<ecs::Transform>("transform_player_name_value", 580.F, 200.F, 0.F)
                                  .with<ecs::Color>("color_player_name_value", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g,
                                                    TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
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
                                  .with<ecs::Color>("color_server_port_value", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g,
                                                    TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
                                  .with<ecs::Text>("server_port_value", m_serverPort, 24U)
                                  .build();
}

void cli::ServerScene::update(const float dt, const eng::WindowSize & /*size*/)
{
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
                m_selectedIndex = (m_selectedIndex == 0) ? m_serverOptions.size() - 1 : m_selectedIndex - 1;
            else if (event.key == eng::Key::Down)
                m_selectedIndex = (m_selectedIndex == m_serverOptions.size() - 1) ? 0 : m_selectedIndex + 1;
            else if (event.key == eng::Key::Enter)
            {
                const std::string &selectedOption = m_serverOptions[m_selectedIndex];
                if (selectedOption == "Connect")
                {
                    if (onConnect)
                        onConnect(m_playerName, m_serverIP, m_serverPort);
                }
                else if (selectedOption == "Back to Menu")
                {
                    if (onBackToMenu)
                        onBackToMenu();
                }
            }
            break;
        case eng::EventType::KeyReleased:
            break;
        default:
            break;
    }
}
