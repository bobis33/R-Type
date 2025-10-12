#include "Client/Scenes/Settings.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include <cmath>

static constexpr eng::Color WHITE = {.r = 255U, .g = 255U, .b = 255U, .a = 255U};

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
    
    registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_title", 200.F, 60.F, 0.F)
        .with<ecs::Color>("color_title", 255U, 80U, 80U, 255U)
        .with<ecs::Text>("title", std::string("SETTINGS"), 64U)
        .build();

    const std::vector<std::string> settingsOptions = {"Audio Volume", "Video Quality", "Controls", "Back to Menu"};
    
    for (size_t i = 0; i < settingsOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_arrow_" + std::to_string(i), 60.F, 200.F + i * 50.F, 0.F)
            .with<ecs::Color>("color_arrow_" + std::to_string(i), 255U, 200U, 0U, 0U)
            .with<ecs::Text>("arrow_" + std::to_string(i), std::string(">"), 32U)
            .build();
            
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_setting_" + std::to_string(i), 100.F, 200.F + i * 50.F, 0.F)
            .with<ecs::Color>("color_setting_" + std::to_string(i), WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("setting_" + settingsOptions[i], settingsOptions[i], 32U)
            .build();
    }
    registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_instruction", 120.F, 400.F, 0.F)
        .with<ecs::Color>("color_instruction", 180U, 180U, 180U, 200U)
        .with<ecs::Text>("instruction", std::string("UP/DOWN to navigate, ENTER to select"), 16U)
        .build();
}

void cli::Settings::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();

    auto &transforms = reg.getAll<ecs::Transform>();
    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();
    auto &audios = reg.getAll<ecs::Audio>();

    for (auto &audio : audios)
    {
        if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
        {
            m_audio->stopAudio(audio.second.id);
        }
    }
    
    for (auto &[entity, text] : texts)
    {
        for (size_t i = 0; i < m_settingsOptions.size(); ++i)
        {
            if (text.content == m_settingsOptions[i])
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
                break;
            }
        }
        if (text.content == ">")
        {
            auto &color = colors.at(entity);
            
            if (text.id == "arrow_" + std::to_string(m_selectedIndex))
            {
                color.a = 255;
            }
            else
            {
                color.a = 0;
            }
        }
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
                if (m_selectedIndex == 0)
                {
                    m_selectedIndex = m_settingsOptions.size() - 1;
                }
                else
                {
                    m_selectedIndex--;
                }
            }
            else if (event.key == eng::Key::Down)
            {
                if (m_selectedIndex == m_settingsOptions.size() - 1)
                {
                    m_selectedIndex = 0;
                }
                else
                {
                    m_selectedIndex++;
                }
            }
            else if (event.key == eng::Key::Enter)
            {
                const std::string &selectedOption = m_settingsOptions[m_selectedIndex];
                
                if (selectedOption == "Back to Menu")
                {
                    onLeave();
                }
            }
            break;

        default:
            break;
    }
}
