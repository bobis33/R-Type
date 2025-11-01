#include <cmath>

#include "ECS/Component.hpp"
#include "RTypeClientSolo/Scenes/ConfigSolo.hpp"
#include "Utils/Common.hpp"

gme::ConfigSolo::ConfigSolo(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
    : AScene(assignedId)
{
    auto &registry = AScene::getRegistry();

    registry.onComponentAdded(
        [&renderer, &registry](const ecs::Entity e, const std::type_info &type)
        {
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
        });

    m_titleEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
            .with<ecs::Color>("color_title", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g,
                              utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
            .with<ecs::Text>("id", std::string("SOLO"), 80U)
            .build();

    for (size_t i = 0; i < m_menuOptions.size(); ++i)
    {
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_menu", 100.F, 200.F + i * 60.F, 0.F)
            .with<ecs::Color>("color_menu", utl::Config::Color::WHITE.r, utl::Config::Color::WHITE.g,
                              utl::Config::Color::WHITE.b, utl::Config::Color::WHITE.a)
            .with<ecs::Text>("menu_" + m_menuOptions[i], m_menuOptions[i], 40U)
            .build();
    }
    m_selectedIndex = 2;
}

void gme::ConfigSolo::update(const float dt, const eng::WindowSize & /*size*/)
{
    auto &reg = getRegistry();

    auto &colors = reg.getAll<ecs::Color>();
    auto &texts = reg.getAll<ecs::Text>();

    m_animationTime += dt;
    m_titlePulseTime += dt;

    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        const float pulse = (std::sin(m_titlePulseTime * 1.2f) + 1.0f) * 0.5f;
        titleColor->r = static_cast<uint8_t>(utl::Config::Color::CYAN_ELECTRIC.r * (0.8f + pulse * 0.2f));
        titleColor->g = static_cast<uint8_t>(utl::Config::Color::CYAN_ELECTRIC.g * (0.8f + pulse * 0.2f));
        titleColor->b = static_cast<uint8_t>(utl::Config::Color::CYAN_ELECTRIC.b * (0.9f + pulse * 0.1f));
    }
    if (auto *titleTransform = reg.getComponent<ecs::Transform>(m_titleEntity))
    {
        titleTransform->y = 60.0f + std::sin(m_titlePulseTime * 0.8f) * 2.0f;
    }
    int i = 0;
    for (auto &[entity, text] : texts)
    {
        if (text.content == "Level easy" || text.content == "Level medium" || text.content == "Go back to menu")
        {
            auto &color = colors.at(entity);

            if (i == m_selectedIndex)
            {
                const float glowIntensity = std::sin(m_animationTime * 2.5f);
                color.r = 0U;
                color.g = static_cast<unsigned char>(191U + (glowIntensity * 50));
                color.b = 255U;
            }
            else
            {
                color.r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
                color.g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
                color.b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
            }

            i++;
        }
    }
    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        const float pulsation = std::sin(m_titlePulseTime * 2.0f) * 0.4f + 0.6f;
        titleColor->r = static_cast<unsigned char>(utl::Config::Color::CYAN_ELECTRIC.r * pulsation);
        titleColor->g = static_cast<unsigned char>(utl::Config::Color::CYAN_ELECTRIC.g * pulsation);
        titleColor->b = static_cast<unsigned char>(utl::Config::Color::CYAN_ELECTRIC.b * pulsation);
    }
    if (auto *fpsText = reg.getComponent<ecs::Text>(m_fpsEntity))
    {
        fpsText->content = "FPS: " + std::to_string(static_cast<int>(1 / dt));
    }
}

void gme::ConfigSolo::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Up)
            {
                m_playMusic = true;
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
                m_playMusic = true;
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
