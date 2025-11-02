#include <cmath>

#include "ECS/Component.hpp"
#include "FlappyBirdClientSolo/Scenes/ConfigFlappy.hpp"
#include "Utils/Common.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

char gme::ConfigFlappy::keyToChar(eng::Key key) const
{
    switch (key)
    {
        case eng::Key::A:
            return 'a';
        case eng::Key::B:
            return 'b';
        case eng::Key::C:
            return 'c';
        case eng::Key::D:
            return 'd';
        case eng::Key::E:
            return 'e';
        case eng::Key::F:
            return 'f';
        case eng::Key::G:
            return 'g';
        case eng::Key::H:
            return 'h';
        case eng::Key::I:
            return 'i';
        case eng::Key::J:
            return 'j';
        case eng::Key::K:
            return 'k';
        case eng::Key::L:
            return 'l';
        case eng::Key::M:
            return 'm';
        case eng::Key::N:
            return 'n';
        case eng::Key::O:
            return 'o';
        case eng::Key::P:
            return 'p';
        case eng::Key::Q:
            return 'q';
        case eng::Key::R:
            return 'r';
        case eng::Key::S:
            return 's';
        case eng::Key::T:
            return 't';
        case eng::Key::U:
            return 'u';
        case eng::Key::V:
            return 'v';
        case eng::Key::W:
            return 'w';
        case eng::Key::X:
            return 'x';
        case eng::Key::Y:
            return 'y';
        case eng::Key::Z:
            return 'z';
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
        default:
            return '\0';
    }
}

std::string &gme::ConfigFlappy::getCurrentEditField()
{
    return m_playerName;
}

void gme::ConfigFlappy::updateValueDisplay()
{
    auto &reg = getRegistry();

    if (auto *playerNameText = reg.getComponent<ecs::Text>(m_playerNameValueEntity))
    {
        playerNameText->content = m_playerName.empty() ? "Player" : m_playerName;
        m_renderer->setTextContent("player_name_value", playerNameText->content);
    }

    if (auto *skinText = reg.getComponent<ecs::Text>(m_skinValueEntity))
    {
        skinText->content = std::to_string(m_skinIndex + 1);
        m_renderer->setTextContent("skin_value", skinText->content);
    }

    if (auto *skinRect = reg.getComponent<ecs::Rect>(m_skinSpriteEntity))
    {
        const std::vector<float> shipLines = {0.0f, 17.0f, 34.0f, 51.0f};
        if (m_skinIndex >= 0 && m_skinIndex < static_cast<int>(shipLines.size()))
        {
            const float skinPosY = shipLines[static_cast<size_t>(m_skinIndex)];
            skinRect->pos_y = skinPosY;
            skinRect->pos_x = 0.0f;
            skinRect->size_x = 33U;
            skinRect->size_y = 17U;
            const std::string spriteName = "player_preview_texture" + std::to_string(m_skinSpriteEntity);
            m_renderer->setSpriteFrame(spriteName, 0, static_cast<int>(skinPosY), 33, 17);
        }
    }
}

gme::ConfigFlappy::ConfigFlappy(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
    : AScene(assignedId), m_renderer(renderer)
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
            .with<ecs::Text>("title", std::string("FLAPPY BIRD"), 80U)
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

    m_playerNameValueEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_player_name_value", 580.F, 200.F, 0.F)
            .with<ecs::Color>("color_player_name_value", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("player_name_value", m_playerName, 24U)
            .build();

    m_skinValueEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_skin_value", 580.F, 260.F, 0.F)
            .with<ecs::Color>("color_skin_value", utl::Config::Color::TEXT_VALUE_COLOR.r,
                              utl::Config::Color::TEXT_VALUE_COLOR.g, utl::Config::Color::TEXT_VALUE_COLOR.b,
                              utl::Config::Color::TEXT_VALUE_COLOR.a)
            .with<ecs::Text>("skin_value", std::to_string(m_skinIndex + 1), 24U)
            .build();

    m_skinSpriteEntity =
        registry.createEntity()
            .with<ecs::Transform>("transform_player_preview", 750.F, 260.F, 0.F)
            .with<ecs::Rect>("player_preview_rect", 0.F, 0.F, 33, 17)
            .with<ecs::Scale>("player_preview_scale", 3.0F, 3.0F)
            .with<ecs::Texture>("player_preview_texture", utl::Path::Texture::TEXTURE_PLAYER)
            .build();

    m_selectedIndex = 3;
    
    updateValueDisplay();
}

void gme::ConfigFlappy::update(const float dt, const eng::WindowSize & /*size*/)
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
        if (text.content == "Player Name" || text.content == "Skin" || text.content == "Start Game" ||
            text.content == "Go back to menu")
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
}

void gme::ConfigFlappy::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Up)
            {
                m_playMusic = true;
                m_selectedIndex = (m_selectedIndex == 0) ? static_cast<int>(m_menuOptions.size()) - 1 : m_selectedIndex - 1;
            }
            else if (event.key == eng::Key::Down)
            {
                m_playMusic = true;
                m_selectedIndex = (m_selectedIndex == static_cast<int>(m_menuOptions.size()) - 1) ? 0 : m_selectedIndex + 1;
            }
            else if (event.key == eng::Key::Enter)
            {
                const std::string &selectedOption = m_menuOptions[m_selectedIndex];
                if (onOptionSelected)
                {
                    onOptionSelected(selectedOption, m_playerName, m_skinIndex);
                }
            }
            else if (event.key == eng::Key::Delete)
            {
                if (m_selectedIndex == 3)
                {
                    std::string &currentField = getCurrentEditField();
                    if (!currentField.empty())
                    {
                        currentField.pop_back();
                        updateValueDisplay();
                    }
                }
            }
            else if (event.key == eng::Key::Left || event.key == eng::Key::Right)
            {
                if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_menuOptions.size()))
                {
                    const std::string &selectedOption = m_menuOptions[m_selectedIndex];
                    if (m_selectedIndex == 2)
                    {
                        if (event.key == eng::Key::Left)
                        {
                            m_skinIndex = (m_skinIndex == 0) ? 3 : m_skinIndex - 1;
                        }
                        else
                        {
                            m_skinIndex = (m_skinIndex == 3) ? 0 : m_skinIndex + 1;
                        }
                        updateValueDisplay();
                    }
                }
            }
            else
            {
                if (m_selectedIndex == 3)
                {
                    if (const char c = keyToChar(event.key); c != '\0')
                    {
                        std::string &currentField = getCurrentEditField();
                        if (currentField.length() < 16)
                        {
                            currentField += c;
                            updateValueDisplay();
                        }
                    }
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

