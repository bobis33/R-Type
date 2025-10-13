#include "Client/Scenes/game/multi/ConfigMulti.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include <cmath>

static constexpr eng::Color CYAN_ELECTRIC = {0U, 191U, 255U, 255U};
static constexpr eng::Color GRAY_BLUE_SUBTLE = {160U, 160U, 160U, 255U};

cli::ConfigMulti::ConfigMulti(const std::shared_ptr<eng::IRenderer> &renderer,
                              const std::shared_ptr<eng::IAudio> &audio)
    : m_audio(audio)
{
    auto &registry = AScene::getRegistry();

    registry.onComponentAdded(
        [&renderer, &audio, &registry](const ecs::Entity e, const std::type_info &type)
        {
            const auto *audioComp = registry.getComponent<ecs::Audio>(e);
            const auto *colorComp = registry.getComponent<ecs::Color>(e);
            const auto *fontComp = registry.getComponent<ecs::Font>(e);
            const auto *rectComp = registry.getComponent<ecs::Rect>(e);
            const auto *scaleComp = registry.getComponent<ecs::Scale>(e);
            const auto *textComp = registry.getComponent<ecs::Text>(e);
            const auto *textureComp = registry.getComponent<ecs::Texture>(e);
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
            else if (type == typeid(ecs::Texture))
            {
                const float scale_x = scaleComp ? scaleComp->x : 1.F;
                const float scale_y = scaleComp ? scaleComp->y : 1.F;

                renderer->createTexture(textureComp->id, textureComp->path);

                if (transform && textureComp)
                {
                    if (rectComp)
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y, scale_x, scale_y, static_cast<int>(rectComp->pos_x),
                                               static_cast<int>(rectComp->pos_y), rectComp->size_x, rectComp->size_y);
                    }
                    else
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y);
                    }
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

    registry.createEntity().with<ecs::Audio>("id_audio", cli::Path::Audio::AUDIO_TITLE, 5.F, true, true).build();

    m_titleEntity = registry.createEntity()
        .with<ecs::Font>("main_font", cli::Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
        .with<ecs::Color>("color_title", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, CYAN_ELECTRIC.a)
        .with<ecs::Text>("title", std::string("MULTIPLAYER"), 72U)
        .build();

    for (size_t i = 0; i < m_menuOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", cli::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_menu_" + std::to_string(i), 100.F, 200.F + i * 60.F, 0.F)
            .with<ecs::Color>("color_menu_" + std::to_string(i), GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g,
                              GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("menu_" + m_menuOptions[i], m_menuOptions[i], 40U)
            .build();
    }

    m_selectedIndex = 2;
}

void cli::ConfigMulti::update(const float dt, const eng::WindowSize & /*size*/)
{
    auto &reg = getRegistry();

    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();
    auto &audios = reg.getAll<ecs::Audio>();

    m_animationTime += dt;
    m_titlePulseTime += dt;

    for (auto &audio : audios)
    {
        if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
        {
            m_audio->stopAudio(audio.second.id);
        }
    }

    int i = 0;
    for (auto &[entity, text] : texts)
    {
        if (text.content == "Create room" || text.content == "Join room" || text.content == "Go back to menu")
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
            i++;
        }
    }
    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        float pulsation = std::sin(m_titlePulseTime * 2.0f) * 0.4f + 0.6f;
        titleColor->r = static_cast<unsigned char>(CYAN_ELECTRIC.r * pulsation);
        titleColor->g = static_cast<unsigned char>(CYAN_ELECTRIC.g * pulsation);
        titleColor->b = static_cast<unsigned char>(CYAN_ELECTRIC.b * pulsation);
    }
    if (auto *fpsText = reg.getComponent<ecs::Text>(m_fpsEntity))
    {
        fpsText->content = "FPS: " + std::to_string(static_cast<int>(1 / dt));
    }
}

void cli::ConfigMulti::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Up)
                m_selectedIndex = (m_selectedIndex == 2) ? 0 : m_selectedIndex + 1;
            else if (event.key == eng::Key::Down)
                m_selectedIndex = (m_selectedIndex == 0) ? 2 : m_selectedIndex - 1;
            else if (event.key == eng::Key::Enter)
            {
                const std::string &selectedOption =
                    m_menuOptions[static_cast<int>(m_menuOptions.size()) - 1 - m_selectedIndex];
                if (onOptionSelected)
                    onOptionSelected(selectedOption);
            }
            break;

        case eng::EventType::KeyReleased:
            m_keysPressed[event.key] = false;
            break;

        default:
            break;
    }
}
