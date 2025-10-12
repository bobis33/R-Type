#include "Client/Scenes/game/multi/ConfigMulti.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include <cmath>

static constexpr eng::Color WHITE = {.r = 255U, .g = 255U, .b = 255U, .a = 255U};

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
    
    registry.createEntity()
        .with<ecs::Font>("main_font", cli::Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_title", 200.F, 60.F, 0.F)
        .with<ecs::Color>("color_title", 255U, 80U, 80U, 255U)
        .with<ecs::Text>("title", std::string("MULTIPLAYER"), 64U)
        .build();

    for (size_t i = 0; i < m_menuOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", cli::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_arrow_" + std::to_string(i), 60.F, 200.F + i * 60.F, 0.F)
            .with<ecs::Color>("color_arrow_" + std::to_string(i), 255U, 200U, 0U, 0U)
            .with<ecs::Text>("arrow_" + std::to_string(i), std::string(">"), 40U)
            .build();
    }

    for (size_t i = 0; i < m_menuOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", cli::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_menu_" + std::to_string(i), 100.F, 200.F + i * 60.F, 0.F)
            .with<ecs::Color>("color_menu_" + std::to_string(i), WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("menu_" + m_menuOptions[i], m_menuOptions[i], 40U)
            .build();
    }
    m_selectedIndex = 2;
}

void cli::ConfigMulti::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();

    auto &transforms = reg.getAll<ecs::Transform>();
    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();
    auto &audios = reg.getAll<ecs::Audio>();

    m_animationTime += dt;
    for (auto &audio : audios)
    {
        if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
        {
            m_audio->stopAudio(audio.second.id);
        }
    }
    for (auto &[entity, text] : texts)
    {
        if (text.content == "MULTIPLAYER")
        {
            auto &color = colors.at(entity);
            float pulsation = std::sin(m_animationTime * 2.0f) * 0.3f + 0.7f;
            color.r = static_cast<unsigned char>(255 * pulsation);
            color.g = static_cast<unsigned char>(80 * pulsation);
            color.b = static_cast<unsigned char>(80 * pulsation);
        }
    }

    size_t i = 0;
    for (auto &[entity, text] : texts)
    {
        if (text.content == "Create room" || text.content == "Join room" || text.content == "Go back to menu")
        {
            auto &color = colors.at(entity);

            if (i == m_selectedIndex)
            {
                color.r = 255;
                color.g = 200;
                color.b = 0;
            }
            else
            {
                color.r = 255;
                color.g = 255;
                color.b = 255;
            }

            i++;
        }
        else if (text.content == ">")
        {
            auto &color = colors.at(entity);
            
            size_t invertedIndex = (m_menuOptions.size() - 1) - m_selectedIndex;
            if (text.id == "arrow_" + std::to_string(invertedIndex))
            {
                float arrowPulse = (std::sin(m_animationTime * 4.0f) + 1.0f) * 0.5f;
                color.r = static_cast<unsigned char>(255);
                color.g = static_cast<unsigned char>(150 + arrowPulse * 105);
                color.b = static_cast<unsigned char>(arrowPulse * 50);
                color.a = 255;
                
                if (auto *arrowTransform = reg.getComponent<ecs::Transform>(entity))
                {
                    float baseX = 60.0f;
                    arrowTransform->x = baseX + std::sin(m_animationTime * 3.0f) * 5.0f;
                }
            } else {
                color.a = 0;
            }
        }
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
            {
                if (m_selectedIndex == 2)
                {
                    m_selectedIndex = 0;
                }
                else
                {
                    m_selectedIndex++;
                }
            }
            else if (event.key == eng::Key::Down)
            {
                if (m_selectedIndex == 0)
                {
                    m_selectedIndex = 2;
                }
                else
                {
                    m_selectedIndex--;
                }
            }
            else if (event.key == eng::Key::Enter)
            {
                const std::string &selectedOption =
                    m_menuOptions[static_cast<int>(m_menuOptions.size()) - 1 - m_selectedIndex];
                if (onOptionSelected)
                {
                    onOptionSelected(selectedOption);
                }
            }
            break;

        case eng::EventType::KeyReleased:
            if (event.key == eng::Key::Up)
            {
                m_keysPressed[eng::Key::Up] = false;
            }
            if (event.key == eng::Key::Down)
            {
                m_keysPressed[eng::Key::Down] = false;
            }
            if (event.key == eng::Key::Left)
            {
                m_keysPressed[eng::Key::Left] = false;
            }
            if (event.key == eng::Key::Right)
            {
                m_keysPressed[eng::Key::Right] = false;
            }
            if (event.key == eng::Key::Space)
            {
                m_keysPressed[eng::Key::Space] = false;
            }
            break;

        default:
            break;
    }
}
