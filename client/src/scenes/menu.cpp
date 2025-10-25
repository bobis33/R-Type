#include <cmath>
#include <ranges>
#include <utility>

#include "Client/Common.hpp"
#include "Client/Scenes/Menu.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"

static constexpr eng::Color CYAN_ELECTRIC = {.r = 0U, .g = 191U, .b = 255U, .a = 255U};
static constexpr eng::Color GRAY_BLUE_SUBTLE = {.r = 160U, .g = 160U, .b = 160U, .a = 255U};
static constexpr eng::Color STAR_BG = {.r = 100U, .g = 100U, .b = 150U, .a = 80U};
static constexpr eng::Color STAR_MID = {.r = 150U, .g = 150U, .b = 200U, .a = 120U};
static constexpr eng::Color SHOOTING_STAR = {.r = 255U, .g = 255U, .b = 200U, .a = 200U};
static constexpr eng::Color CYAN_ELECTRIC_PARTICLES = {.r = 0U, .g = 191U, .b = 255U, .a = 100U};
static constexpr eng::Color CYAN_ELECTRIC_FOREGROUND = {.r = 0U, .g = 191U, .b = 255U, .a = 180U};

cli::Menu::Menu(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio)
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

    m_menuBgmEntity =
        registry.createEntity().with<ecs::Audio>("menu_bgm", Path::Audio::AUDIO_TITLE, 1.F, false, false).build();
    m_menuBgmName = "menu_bgm" + std::to_string(m_menuBgmEntity);

    m_selectionSoundEntity =
        registry.createEntity().with<ecs::Audio>("menu_input", Path::Audio::AUDIO_INPUT, 5.F, false, false).build();
    m_selectionSoundName = "menu_input" + std::to_string(m_selectionSoundEntity);

    startMenuMusicOnce();

    m_titleEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
            .with<ecs::Color>("color_title", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, CYAN_ELECTRIC.a)
            .with<ecs::Text>("id", std::string("RTYPE"), 72U)
            .build();

    for (size_t i = 0; i < m_menuOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_menu", 100.F, 200.F + i * 60.F, 0.F)
            .with<ecs::Color>("color_menu", GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g, GRAY_BLUE_SUBTLE.b,
                              GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("menu_" + m_menuOptions[i], m_menuOptions[i], 32U)
            .build();
    }
    const int screenWidth = renderer->getWindowSize().width;
    const int screenHeight = renderer->getWindowSize().height;

    for (int i = 0; i < 40; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_background")
            .with<ecs::Transform>("star_bg_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_bg_color", STAR_BG.r, STAR_BG.g, STAR_BG.b, STAR_BG.a)
            .with<ecs::Velocity>("star_bg_vel", -10.0f, 0.0f)
            .build();
    }
    for (int i = 0; i < 25; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_middle")
            .with<ecs::Transform>("star_mid_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_mid_color", STAR_MID.r, STAR_MID.g, STAR_MID.b, STAR_MID.a)
            .with<ecs::Velocity>("star_mid_vel", -25.0f, 0.0f)
            .build();
    }
    for (int i = 0; i < 15; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_foreground")
            .with<ecs::Transform>("star_fg_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_fg_color", CYAN_ELECTRIC_FOREGROUND.r, CYAN_ELECTRIC_FOREGROUND.g,
                              CYAN_ELECTRIC_FOREGROUND.b, CYAN_ELECTRIC_FOREGROUND.a)
            .with<ecs::Velocity>("star_fg_vel", -50.0f, 0.0f)
            .build();
    }
    for (int i = 0; i < 8; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("shooting_star")
            .with<ecs::Transform>("shooting_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("shooting_color", SHOOTING_STAR.r, SHOOTING_STAR.g, SHOOTING_STAR.b, SHOOTING_STAR.a)
            .with<ecs::Velocity>("shooting_vel", -80.0f, static_cast<float>((std::rand() % 20) - 10))
            .build();
    }
    for (int i = 0; i < 12; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("cyan_particle")
            .with<ecs::Transform>("cyan_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("cyan_color", CYAN_ELECTRIC_PARTICLES.r, CYAN_ELECTRIC_PARTICLES.g,
                              CYAN_ELECTRIC_PARTICLES.b, CYAN_ELECTRIC_PARTICLES.a)
            .with<ecs::Velocity>("cyan_vel", -35.0f, static_cast<float>((std::rand() % 10) - 5))
            .build();
    }

    std::string contributorsText = "Contributors ";
    for (size_t i = 0; i < m_contributors.size(); ++i)
    {
        contributorsText += m_contributors[i];
        if (i < m_contributors.size() - 1)
        {
            contributorsText += " ";
        }
    }
    m_contributorsEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_contributors", static_cast<float>(renderer->getWindowSize().width * 0.9F),
                                  static_cast<float>(renderer->getWindowSize().height * 0.9F))
            .with<ecs::Color>("color_contributors", GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g, GRAY_BLUE_SUBTLE.b,
                              GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("contributors_text", contributorsText, 24U)
            .build();

    m_selectedIndex = 2;
}

void cli::Menu::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();
    startMenuMusicOnce();

    auto &transforms = reg.getAll<ecs::Transform>();
    auto &colors = reg.getAll<ecs::Color>();
    m_animationTime += dt;
    m_titlePulseTime += dt;

    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        const float pulse = (std::sin(m_titlePulseTime * 1.2f) + 1.0f) * 0.5f;
        titleColor->r = static_cast<uint8_t>(CYAN_ELECTRIC.r * (0.8f + pulse * 0.2f));
        titleColor->g = static_cast<uint8_t>(CYAN_ELECTRIC.g * (0.8f + pulse * 0.2f));
        titleColor->b = static_cast<uint8_t>(CYAN_ELECTRIC.b * (0.9f + pulse * 0.1f));
    }

    if (auto *titleTransform = reg.getComponent<ecs::Transform>(m_titleEntity))
    {
        titleTransform->y = 60.0f + std::sin(m_titlePulseTime * 0.8f) * 2.0f;
    }

    for (const auto &entity : reg.getAll<ecs::Pixel>() | std::views::keys)
    {
        if (auto *transform = reg.getComponent<ecs::Transform>(entity))
        {
            if (const auto *velocity = reg.getComponent<ecs::Velocity>(entity))
            {
                transform->x += velocity->x * dt;
                transform->y += velocity->y * dt;
                if (transform->x < -10.0f)
                {
                    transform->x = static_cast<float>(size.width + (std::rand() % 100));
                    transform->y = static_cast<float>(std::rand() % size.height);
                }
                else if (transform->x > size.width + 10.0f)
                {
                    transform->x = -10.0f;
                }

                if (transform->y < -10.0f || transform->y > size.height + 10.0f)
                {
                    transform->y = static_cast<float>(std::rand() % size.height);
                }
            }
        }
    }

    auto &texts = reg.getAll<ecs::Text>();
    size_t i = 0;
    for (auto &[entity, text] : texts)
    {
        if (text.content == "Solo" || text.content == "Multi" || text.content == "Settings")
        {
            auto &color = colors.at(entity);

            if (std::cmp_equal(i, m_selectedIndex))
            {
                const float glowIntensity = std::sin(m_animationTime * 2.5f);
                color.r = 0U;
                color.g = static_cast<unsigned char>(191U + (glowIntensity * 50));
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
    m_contributorsOffset += dt * 50.0f;
    if (auto *contributorsTransform = reg.getComponent<ecs::Transform>(m_contributorsEntity))
    {
        contributorsTransform->x = (size.width * 0.9f) - m_contributorsOffset;

        if (contributorsTransform->x < -(size.width * 0.9F))
        {
            m_contributorsOffset = 0.0f;
            contributorsTransform->x = size.width * 0.9f;
        }
    }
    if (auto *fpsText = reg.getComponent<ecs::Text>(m_fpsEntity))
    {
        fpsText->content = "FPS " + std::to_string(static_cast<int>(1 / dt));
    }
}

void cli::Menu::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
        {
            const int previousIndex = m_selectedIndex;
            bool handledNavigation = false;

            if (event.key == eng::Key::Up)
            {
                handledNavigation = true;
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
                handledNavigation = true;
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
                stopMenuMusic();
                const std::string &selectedOption =
                    m_menuOptions[static_cast<int>(m_menuOptions.size()) - 1 - m_selectedIndex];
                if (onOptionSelected)
                {
                    onOptionSelected(selectedOption);
                }
            }

            if (handledNavigation && m_selectedIndex != previousIndex)
            {
                playSelectionSound();
            }
            break;
        }

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

void cli::Menu::playSelectionSound()
{
    if (m_selectionSoundName.empty())
        return;

    m_audio->stopAudio(m_selectionSoundName);
    m_audio->playAudio(m_selectionSoundName);
}

void cli::Menu::stopMenuMusic()
{
    if (m_menuBgmName.empty())
        return;

    auto &reg = getRegistry();
    if (auto *audioComp = reg.getComponent<ecs::Audio>(m_menuBgmEntity))
    {
        audioComp->play = false;
    }

    m_audio->stopAudio(m_menuBgmName);
    m_hasStartedMenuMusic = false;
}

void cli::Menu::startMenuMusicOnce()
{
    if (m_hasStartedMenuMusic || m_menuBgmName.empty())
        return;

    m_audio->stopAudio(m_menuBgmName);
    m_audio->playAudio(m_menuBgmName);
    m_hasStartedMenuMusic = true;
}
