#include "Client/Scenes/Menu.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include <cmath>

static constexpr eng::Color WHITE = {.r = 255U, .g = 255U, .b = 255U, .a = 255U};
static constexpr eng::Color CYAN_ELECTRIC = {0U, 191U, 255U, 255U};
static constexpr eng::Color GRAY_BLUE_SUBTLE = {160U, 160U, 160U, 255U};
static constexpr eng::Color VIOLET_LIGHT = {154U, 109U, 255U, 255U};
static constexpr eng::Color SPACE_BG = {0U, 0U, 30U, 255U};


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

    registry.createEntity().with<ecs::Audio>("id_audio", Path::Audio::AUDIO_TITLE, 5.F, true, true).build();

    m_titleEntity = registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_title", 250.F, 60.F, 0.F)
        .with<ecs::Color>("color_title", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, CYAN_ELECTRIC.a)
        .with<ecs::Text>("id", std::string("RTYPE"), 72U)
        .build();

    for (size_t i = 0; i < m_menuOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_menu", 100.F, 200.F + i * 60.F, 0.F)
            .with<ecs::Color>("color_menu", GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g, GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("menu_" + m_menuOptions[i], m_menuOptions[i], 32U)
            .build();
    }
    
    const int screenWidth = 960;
    const int screenHeight = 540;

    for (int i = 0; i < 40; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_background")
            .with<ecs::Transform>("star_bg_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_bg_color", 100U, 100U, 150U, 80U)
            .with<ecs::Velocity>("star_bg_vel", -10.0f, 0.0f)
            .build();
    }

    for (int i = 0; i < 25; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_middle")
            .with<ecs::Transform>("star_mid_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_mid_color", 150U, 150U, 200U, 120U)
            .with<ecs::Velocity>("star_mid_vel", -25.0f, 0.0f)
            .build();
    }

    for (int i = 0; i < 15; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_foreground")
            .with<ecs::Transform>("star_fg_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_fg_color", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, 180U)
            .with<ecs::Velocity>("star_fg_vel", -50.0f, 0.0f)
            .build();
    }

    for (int i = 0; i < 8; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("shooting_star")
            .with<ecs::Transform>("shooting_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("shooting_color", 255U, 255U, 200U, 200U)
            .with<ecs::Velocity>("shooting_vel", -80.0f, static_cast<float>((std::rand() % 20) - 10))
            .build();
    }

    for (int i = 0; i < 12; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("cyan_particle")
            .with<ecs::Transform>("cyan_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("cyan_color", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, 100U)
            .with<ecs::Velocity>("cyan_vel", -35.0f, static_cast<float>((std::rand() % 10) - 5))
            .build();
    }
    
    registry.createEntity()
        .with<ecs::Transform>("moon_earth_transform", 100.0f, 0.0f, 0.0f)
        .with<ecs::Color>("moon_earth_color", 255U, 255U, 255U, 160U)
        .with<ecs::Scale>("moon_earth_scale", 1.0f, 1.0f)
        .with<ecs::Texture>("moon_earth_texture", Path::Texture::TEXTURE_MOON_EARTH)
        .with<ecs::Velocity>("moon_earth_vel", -5.0f, 0.0f)
        .build();

    registry.createEntity()
        .with<ecs::Transform>("moon_back_transform", 200.0f, 20.0f, 0.0f)
        .with<ecs::Color>("moon_back_color", 255U, 255U, 255U, 200U)
        .with<ecs::Scale>("moon_back_scale", 0.9f, 0.9f)
        .with<ecs::Texture>("moon_back_texture", Path::Texture::TEXTURE_MOON_BACK)
        .with<ecs::Velocity>("moon_back_vel", -15.0f, 0.0f)
        .build();

    registry.createEntity()
        .with<ecs::Transform>("moon_mid_transform", 400.0f, 40.0f, 0.0f)
        .with<ecs::Color>("moon_mid_color", 255U, 255U, 255U, 220U)
        .with<ecs::Scale>("moon_mid_scale", 1.0f, 1.0f)
        .with<ecs::Texture>("moon_mid_texture", Path::Texture::TEXTURE_MOON_MID)
        .with<ecs::Velocity>("moon_mid_vel", -30.0f, 0.0f)
        .build();

    registry.createEntity()
        .with<ecs::Transform>("moon_floor_transform", 0.0f, 200.0f, 0.0f)
        .with<ecs::Color>("moon_floor_color", 255U, 255U, 255U, 255U)
        .with<ecs::Scale>("moon_floor_scale", 1.2f, 0.8f)
        .with<ecs::Texture>("moon_floor_texture", Path::Texture::TEXTURE_MOON_FLOOR)
        .with<ecs::Velocity>("moon_floor_vel", -60.0f, 0.0f)
        .build();

    registry.createEntity()
        .with<ecs::Transform>("moon_front_transform", 600.0f, 120.0f, 0.0f)
        .with<ecs::Color>("moon_front_color", 255U, 255U, 255U, 240U)
        .with<ecs::Scale>("moon_front_scale", 0.8f, 0.8f)
        .with<ecs::Texture>("moon_front_texture", Path::Texture::TEXTURE_MOON_FRONT)
        .with<ecs::Velocity>("moon_front_vel", -80.0f, 0.0f)
        .build();

    std::string contributorsText = "Contributors ";
    for (size_t i = 0; i < m_contributors.size(); ++i)
    {
        contributorsText += m_contributors[i];
        if (i < m_contributors.size() - 1)
            contributorsText += " ";
    }
    m_contributorsEntity = registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_contributors", 800.F, 500.F, 0.F)
        .with<ecs::Color>("color_contributors", GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g, GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
        .with<ecs::Text>("contributors_text", contributorsText, 24U)
        .build();
        
    m_selectedIndex = 2;
}

void cli::Menu::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();

    auto &transforms = reg.getAll<ecs::Transform>();
    auto &colors = reg.getAll<ecs::Color>();
    auto &audios = reg.getAll<ecs::Audio>();

    m_animationTime += dt;
    m_titlePulseTime += dt;

    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        float pulse = (std::sin(m_titlePulseTime * 1.2f) + 1.0f) * 0.5f;
        titleColor->r = static_cast<uint8_t>(CYAN_ELECTRIC.r * (0.8f + pulse * 0.2f));
        titleColor->g = static_cast<uint8_t>(CYAN_ELECTRIC.g * (0.8f + pulse * 0.2f));
        titleColor->b = static_cast<uint8_t>(CYAN_ELECTRIC.b * (0.9f + pulse * 0.1f));
    }
    
    if (auto *titleTransform = reg.getComponent<ecs::Transform>(m_titleEntity))
    {
        titleTransform->y = 60.0f + std::sin(m_titlePulseTime * 0.8f) * 2.0f;
    }

    for (auto &audio : audios)
    {
        if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
        {
            m_audio->stopAudio(audio.second.id);
        }
    }
    for (auto &[entity, pixel] : reg.getAll<ecs::Pixel>())
    {
        if (auto *transform = reg.getComponent<ecs::Transform>(entity))
        {
            if (auto *velocity = reg.getComponent<ecs::Velocity>(entity))
            {
                transform->x += velocity->x * dt;
                transform->y += velocity->y * dt;
                if (transform->x < -10.0f)
                {
                    transform->x = static_cast<float>(size.width + std::rand() % 100);
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
    auto &textures = reg.getAll<ecs::Texture>();
    for (auto &[entity, texture] : textures)
    {
        if (texture.path.find("moon_") != std::string::npos)
        {
            if (auto *transform = reg.getComponent<ecs::Transform>(entity))
            {
                if (auto *velocity = reg.getComponent<ecs::Velocity>(entity))
                {
                    transform->x += velocity->x * dt;
                    float textureWidth = 960.0f;
                    if (auto *scale = reg.getComponent<ecs::Scale>(entity))
                    {
                        textureWidth *= scale->x;
                    }
                    if (transform->x < -textureWidth)
                    {
                        transform->x = static_cast<float>(size.width);
                    }
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
    m_contributorsOffset += dt * 50.0f;
    if (auto *contributorsTransform = reg.getComponent<ecs::Transform>(m_contributorsEntity))
    {
        contributorsTransform->x = 800.0f - m_contributorsOffset;
        
        if (contributorsTransform->x < -400.0f)
        {
            m_contributorsOffset = 0.0f;
            contributorsTransform->x = 800.0f;
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