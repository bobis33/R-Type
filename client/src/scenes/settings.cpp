#include "Client/Client.hpp"
#include "Client/Scenes/Settings.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "Utils/Common.hpp"

cli::Settings::Settings(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                        const std::shared_ptr<eng::IAudio> &audio, const AppConfig &config)
    : AScene(assignedId), m_renderer(renderer), m_audio(audio), m_appConfig(config)
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

    m_titleEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
            .with<ecs::Color>("color_title", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g, utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
            .with<ecs::Text>("title", std::string("SETTINGS"), 72U)
            .build();

    for (size_t i = 0; i < m_settingsOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_setting_" + std::to_string(i), 100.F, 200.F + i * 50.F, 0.F)
            .with<ecs::Color>("color_setting_" + std::to_string(i), utl::Config::Color::GRAY_BLUE_SUBTLE.r, utl::Config::Color::GRAY_BLUE_SUBTLE.g,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.b, utl::Config::Color::GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("setting_" + m_settingsOptions[i], m_settingsOptions[i], 32U)
            .build();
    }
    m_volumeValueEntity = registry.createEntity()
                              .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                              .with<ecs::Transform>("transform_volume_value", 580.F, 200.F, 0.F)
                              .with<ecs::Color>("color_volume_value", utl::Config::Color::TEXT_VALUE_COLOR.r, utl::Config::Color::TEXT_VALUE_COLOR.g,
                                                utl::Config::Color::TEXT_VALUE_COLOR.b, utl::Config::Color::TEXT_VALUE_COLOR.a)
                              .with<ecs::Text>("volume_value", std::string("50"), 24U)
                              .build();
    m_qualityValueEntity = registry.createEntity()
                               .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                               .with<ecs::Transform>("transform_quality_value", 580.F, 250.F, 0.F)
                               .with<ecs::Color>("color_quality_value", utl::Config::Color::TEXT_VALUE_COLOR.r, utl::Config::Color::TEXT_VALUE_COLOR.g,
                                                 utl::Config::Color::TEXT_VALUE_COLOR.b, utl::Config::Color::TEXT_VALUE_COLOR.a)
                               .with<ecs::Text>("quality_value", std::string("Medium"), 24U)
                               .build();
    m_controlValueEntity = registry.createEntity()
                               .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                               .with<ecs::Transform>("transform_control_value", 580.F, 300.F, 0.F)
                               .with<ecs::Color>("color_control_value", utl::Config::Color::TEXT_VALUE_COLOR.r, utl::Config::Color::TEXT_VALUE_COLOR.g,
                                                 utl::Config::Color::TEXT_VALUE_COLOR.b, utl::Config::Color::TEXT_VALUE_COLOR.a)
                               .with<ecs::Text>("control_value", std::string("WASD"), 24U)
                               .build();

    m_skinSpriteEntity = registry.createEntity()
                             .with<ecs::Transform>("transform_skin_sprite", 580.F, 345.F, 0.F)
                             .with<ecs::Scale>("scale_skin_sprite", 2.0f, 2.0f)
                             .with<ecs::Color>("color_skin_sprite", utl::Config::Color::WHITE.r, utl::Config::Color::WHITE.g, utl::Config::Color::WHITE.b, utl::Config::Color::WHITE.a)
                             .with<ecs::Rect>("rect_skin_sprite", 0.0f, 0.0f, 33, 17)
                             .with<ecs::Texture>("skin_sprite", "assets/sprites/r-typesheet42.gif")
                             .build();
    registry.createEntity()
        .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_instruction", 80.F, 480.F, 0.F)
        .with<ecs::Color>("color_instruction", utl::Config::Color::INFO_TEXT_COLOR.r, utl::Config::Color::INFO_TEXT_COLOR.g, utl::Config::Color::INFO_TEXT_COLOR.b,
                          utl::Config::Color::INFO_TEXT_COLOR.a)
        .with<ecs::Text>("instruction", std::string("UP/DOWN navigate, LEFT/RIGHT change, ESC back"), 16U)
        .build();

    m_selectionSoundEntity = registry.createEntity()
                                 .with<ecs::Audio>("settings_input", utl::Path::Audio::AUDIO_INPUT, 8.F, false, false)
                                 .build();
    m_selectionSoundName = "settings_input" + std::to_string(m_selectionSoundEntity);

    m_selectedIndex = 0;
    loadFromConfig();
}

void cli::Settings::update(const float dt, const eng::WindowSize & /*size*/)
{
    auto &reg = getRegistry();
    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();
    auto &audios = reg.getAll<ecs::Audio>();

    m_animationTime += dt;
    m_titlePulseTime += dt;
    for (auto &val : audios | std::views::values)
    {
        if (!val.play && (m_audio->isPlaying(val.id) == eng::Status::Playing))
            m_audio->stopAudio(val.id);
    }
    for (auto &[entity, text] : texts)
    {
        for (size_t i = 0; i < m_settingsOptions.size(); ++i)
        {
            if (text.id == "setting_" + m_settingsOptions[i])
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
                    color.r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
                    color.g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
                    color.b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
                }
                break;
            }
        }
    }
    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        float pulsation = std::sin(m_titlePulseTime * 2.0f) * 0.4f + 0.6f;
        titleColor->r = static_cast<unsigned char>(utl::Config::Color::CYAN_ELECTRIC.r * pulsation);
        titleColor->g = static_cast<unsigned char>(utl::Config::Color::CYAN_ELECTRIC.g * pulsation);
        titleColor->b = static_cast<unsigned char>(utl::Config::Color::CYAN_ELECTRIC.b * pulsation);
    }

    updateSettingsDisplay();
}

void cli::Settings::updateSettingsDisplay()
{
    auto &registry = getRegistry();

    if (auto *volumeValueText = registry.getComponent<ecs::Text>(m_volumeValueEntity))
        volumeValueText->content = std::to_string(m_audioVolume);
    if (auto *qualityValueText = registry.getComponent<ecs::Text>(m_qualityValueEntity))
    {
        const std::vector<std::string> fpsOptions = {"60 FPS", "144 FPS", "240 FPS"};
        qualityValueText->content = fpsOptions[static_cast<size_t>(m_videoQuality)];
    }
    if (auto *controlValueText = registry.getComponent<ecs::Text>(m_controlValueEntity))
    {
        const std::vector<std::string> controlSchemes = {"WASD", "ZQSD", "Arrows"};
        controlValueText->content = controlSchemes[static_cast<size_t>(m_controlScheme)];
    }
    if (auto *skinRect = registry.getComponent<ecs::Rect>(m_skinSpriteEntity))
    {
        const std::vector shipLines = {0.0f, 17.0f, 34.0f, 51.0f, 68.0f};

        skinRect->pos_y = shipLines[static_cast<size_t>(m_skinIndex)];
        skinRect->pos_x = 0.0f;
        skinRect->size_x = 33U;
        skinRect->size_y = 17U;
    }
}

void cli::Settings::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Escape)
            {
                onLeave();
            }
            else if (event.key == eng::Key::Up)
            {
                m_selectedIndex = (m_selectedIndex == 0) ? m_settingsOptions.size() - 1 : m_selectedIndex - 1;
                playInputSound();
            }
            else if (event.key == eng::Key::Down)
            {
                m_selectedIndex = (m_selectedIndex == m_settingsOptions.size() - 1) ? 0 : m_selectedIndex + 1;
                playInputSound();
            }
            else if (event.key == eng::Key::Enter)
            {
                if (const std::string &selectedOption = m_settingsOptions[m_selectedIndex];
                    selectedOption == "Back to Menu")
                    onLeave();
            }
            else if (event.key == eng::Key::Left || event.key == eng::Key::Right)
            {

                if (const std::string &selectedOption = m_settingsOptions[m_selectedIndex];
                    selectedOption == "Audio Volume")
                {
                    const float newVolume = m_audioVolume + ((event.key == eng::Key::Right) ? 0.01F : -0.01F);
                    m_audioVolume = (std::max)(0.0F, (std::min)(10.0F, newVolume));
                    const_cast<AppConfig &>(m_appConfig).audioVolume = m_audioVolume;
                }
                else if (selectedOption == "FPS")
                {
                    if (event.key == eng::Key::Left)
                        m_videoQuality = (m_videoQuality == 0) ? 2 : m_videoQuality - 1;
                    else
                        m_videoQuality = (m_videoQuality == 2) ? 0 : m_videoQuality + 1;
                    const_cast<AppConfig &>(m_appConfig).videoQuality = m_videoQuality;
                    applyVideoQuality();
                }
                else if (selectedOption == "Controls")
                {
                    if (event.key == eng::Key::Left)
                        m_controlScheme = (m_controlScheme == 0) ? 2 : m_controlScheme - 1;
                    else
                        m_controlScheme = (m_controlScheme == 2) ? 0 : m_controlScheme + 1;
                    const_cast<AppConfig &>(m_appConfig).controlScheme = m_controlScheme;
                }
                else if (selectedOption == "Skin")
                {
                    if (event.key == eng::Key::Left)
                        m_skinIndex = (m_skinIndex == 0) ? 4 : m_skinIndex - 1;
                    else
                        m_skinIndex = (m_skinIndex == 4) ? 0 : m_skinIndex + 1;
                    const_cast<AppConfig &>(m_appConfig).skinIndex = m_skinIndex;

                    applySkinChange();
                }
                updateSettingsDisplay();
            }
            break;
        default:
            break;
    }
}

void cli::Settings::loadFromConfig()
{
    m_audioVolume = m_appConfig.audioVolume;
    m_videoQuality = m_appConfig.videoQuality;
    m_controlScheme = m_appConfig.controlScheme;
    m_skinIndex = m_appConfig.skinIndex;

    updateSettingsDisplay();
    applyVideoQuality();
}

void cli::Settings::applyVideoQuality() {  }

void cli::Settings::applySkinChange() {}

void cli::Settings::playInputSound() const
{
    if (m_selectionSoundName.empty())
        return;

    m_audio->stopAudio(m_selectionSoundName);
    m_audio->playAudio(m_selectionSoundName);
}
