#include "Client/Scenes/Settings.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include <cmath>
#include <algorithm>

// 🎨 Couleurs R-Type
static constexpr eng::Color CYAN_ELECTRIC = {0U, 191U, 255U, 255U};     // #00BFFF - Cyan électrique principal
static constexpr eng::Color GRAY_BLUE_SUBTLE = {160U, 160U, 160U, 255U}; // #A0A0A0 - Gris bleuté subtil
static constexpr eng::Color TEXT_VALUE_COLOR = {200U, 200U, 255U, 255U}; // Bleu clair pour les valeurs de droite
static constexpr eng::Color INFO_TEXT_COLOR = {180U, 180U, 180U, 200U};  // Gris doux pour l’instruction

cli::Settings::Settings(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio)
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
        .with<ecs::Transform>("transform_title", 200.F, 60.F, 0.F)
        .with<ecs::Color>("color_title", CYAN_ELECTRIC.r, CYAN_ELECTRIC.g, CYAN_ELECTRIC.b, CYAN_ELECTRIC.a)
        .with<ecs::Text>("title", std::string("SETTINGS"), 72U)
        .build();

    for (size_t i = 0; i < m_settingsOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_setting_" + std::to_string(i), 100.F, 200.F + i * 50.F, 0.F)
            .with<ecs::Color>("color_setting_" + std::to_string(i),
                              GRAY_BLUE_SUBTLE.r, GRAY_BLUE_SUBTLE.g, GRAY_BLUE_SUBTLE.b, GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("setting_" + m_settingsOptions[i], m_settingsOptions[i], 32U)
            .build();
    }

    // 🎚️ Valeurs de droite
    m_volumeValueEntity = registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_volume_value", 580.F, 200.F, 0.F)
        .with<ecs::Color>("color_volume_value", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g, TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
        .with<ecs::Text>("volume_value", std::string("50"), 24U)
        .build();

    m_qualityValueEntity = registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_quality_value", 580.F, 250.F, 0.F)
        .with<ecs::Color>("color_quality_value", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g, TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
        .with<ecs::Text>("quality_value", std::string("Medium"), 24U)
        .build();

    m_controlValueEntity = registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_control_value", 580.F, 300.F, 0.F)
        .with<ecs::Color>("color_control_value", TEXT_VALUE_COLOR.r, TEXT_VALUE_COLOR.g, TEXT_VALUE_COLOR.b, TEXT_VALUE_COLOR.a)
        .with<ecs::Text>("control_value", std::string("WASD"), 24U)
        .build();

    // 💬 Instructions
    registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_instruction", 80.F, 400.F, 0.F)
        .with<ecs::Color>("color_instruction", INFO_TEXT_COLOR.r, INFO_TEXT_COLOR.g, INFO_TEXT_COLOR.b, INFO_TEXT_COLOR.a)
        .with<ecs::Text>("instruction", std::string("UP/DOWN navigate, ENTER select, ESC back"), 16U)
        .build();

    m_selectedIndex = 0;
}

void cli::Settings::update(const float dt, const eng::WindowSize & /*size*/)
{
    auto &reg = getRegistry();
    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();
    auto &audios = reg.getAll<ecs::Audio>();

    m_animationTime += dt;
    m_titlePulseTime += dt;

    // 🎵 Musique (arrêt automatique si non jouée)
    for (auto &audio : audios)
    {
        if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
            m_audio->stopAudio(audio.second.id);
    }

    // 🔄 Gestion des couleurs dynamiques pour les options
    for (auto &[entity, text] : texts)
    {
        // Vérifier si c'est une option de menu en utilisant l'id
        for (size_t i = 0; i < m_settingsOptions.size(); ++i)
        {
            if (text.id == "setting_" + m_settingsOptions[i])
            {
                auto &color = colors.at(entity);

                if (i == m_selectedIndex)
                {
                    // Animation cyan électrique avec pulsation (comme dans les autres menus)
                    float glowIntensity = std::sin(m_animationTime * 2.5f);
                    color.r = 0U;
                    color.g = static_cast<unsigned char>(191U + glowIntensity * 50);
                    color.b = 255U;
                }
                else
                {
                    // Couleur grise subtile pour les options non sélectionnées
                    color.r = GRAY_BLUE_SUBTLE.r;
                    color.g = GRAY_BLUE_SUBTLE.g;
                    color.b = GRAY_BLUE_SUBTLE.b;
                }
                break; // Sortir de la boucle interne une fois qu'on a trouvé la correspondance
            }
        }
    }

    // Animation du titre "SETTINGS" avec pulsation cyan électrique
    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        float pulsation = std::sin(m_titlePulseTime * 2.0f) * 0.4f + 0.6f;
        titleColor->r = static_cast<unsigned char>(CYAN_ELECTRIC.r * pulsation);
        titleColor->g = static_cast<unsigned char>(CYAN_ELECTRIC.g * pulsation);
        titleColor->b = static_cast<unsigned char>(CYAN_ELECTRIC.b * pulsation);
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
        const std::vector<std::string> qualities = {"Low", "Medium", "High"};
        qualityValueText->content = qualities[m_videoQuality];
    }

    if (auto *controlValueText = registry.getComponent<ecs::Text>(m_controlValueEntity))
    {
        const std::vector<std::string> controlSchemes = {"WASD", "ZQSD", "Arrows"};
        controlValueText->content = controlSchemes[m_controlScheme];
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
                m_selectedIndex = (m_selectedIndex == 0) ? m_settingsOptions.size() - 1 : m_selectedIndex - 1;
            else if (event.key == eng::Key::Down)
                m_selectedIndex = (m_selectedIndex == m_settingsOptions.size() - 1) ? 0 : m_selectedIndex + 1;
            else if (event.key == eng::Key::Enter)
            {
                const std::string &selectedOption = m_settingsOptions[m_selectedIndex];
                if (selectedOption == "Back to Menu")
                    onLeave();
            }
            else if (event.key == eng::Key::Left || event.key == eng::Key::Right)
            {
                const std::string &selectedOption = m_settingsOptions[m_selectedIndex];

                if (selectedOption == "Audio Volume")
                {
                    int newVolume = m_audioVolume + ((event.key == eng::Key::Right) ? 10 : -10);
                    m_audioVolume = std::max(0, std::min(100, newVolume));
                }
                else if (selectedOption == "Video Quality")
                {
                    if (event.key == eng::Key::Left)
                        m_videoQuality = (m_videoQuality == 0) ? 2 : m_videoQuality - 1;
                    else
                        m_videoQuality = (m_videoQuality == 2) ? 0 : m_videoQuality + 1;
                }
                else if (selectedOption == "Controls")
                {
                    if (event.key == eng::Key::Left)
                        m_controlScheme = (m_controlScheme == 0) ? 2 : m_controlScheme - 1;
                    else
                        m_controlScheme = (m_controlScheme == 2) ? 0 : m_controlScheme + 1;
                }
            }
            break;
        default:
            break;
    }
}
