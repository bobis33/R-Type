#include "Client/Scenes/Menu.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include <cmath>
#include <iostream>

static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};
static constexpr eng::Color CYAN_GLOW = {0U, 200U, 255U, 255U};
static constexpr eng::Color GRAY = {160U, 160U, 160U, 255U};

namespace cli
{
    Menu::Menu(const std::shared_ptr<eng::IRenderer> &renderer,
               const std::shared_ptr<eng::IAudio> &audio,
               eng::SceneManager *sceneManager)
        : m_sceneManager(sceneManager)
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

                if (type == typeid(ecs::Text) && textComp && transform && fontComp)
                {
                    renderer->createFont(fontComp->id, fontComp->path);
                    renderer->createText({
                        .font_name = fontComp->id,
                        .color = {.r = colorComp->r, .g = colorComp->g, .b = colorComp->b, .a = colorComp->a},
                        .content = textComp->content,
                        .size = textComp->font_size,
                        .x = transform->x,
                        .y = transform->y,
                        .name = textComp->id});
                }
                else if (type == typeid(ecs::Texture) && textureComp)
                {
                    const float sx = scaleComp ? scaleComp->x : 1.F;
                    const float sy = scaleComp ? scaleComp->y : 1.F;
                    renderer->createTexture(textureComp->id, textureComp->path);

                    if (transform)
                    {
                        if (rectComp)
                        {
                            renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id,
                                                   transform->x, transform->y, sx, sy,
                                                   static_cast<int>(rectComp->pos_x),
                                                   static_cast<int>(rectComp->pos_y),
                                                   rectComp->size_x, rectComp->size_y);
                        }
                        else
                        {
                            renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id,
                                                   transform->x, transform->y, sx, sy);
                        }
                    }
                }
                else if (type == typeid(ecs::Audio) && audioComp)
                {
                    audio->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                       audioComp->id + std::to_string(e));
                }
            });

        createMenuEntities();
    }

    void Menu::createMenuEntities()
    {
        auto &reg = getRegistry();
        constexpr float W = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_WIDTH);
        constexpr float H = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_HEIGHT);

        reg.createEntity()
            .with<ecs::Audio>("menu_music", Path::Audio::AUDIO_TITLE, 5.F, true, true)
            .build();

        const float logoX = W / 2.f - 128.f;
        const float logoY = H / 2.f - 180.f;
        m_logoEntity = reg.createEntity()
                           .with<ecs::Transform>("logo_transform", logoX, logoY, 0.F)
                           .with<ecs::Scale>("logo_scale", 0.8F, 0.8F)
                           .with<ecs::Texture>("rtype_logo", Path::Texture::TEXTURE_LOGO)
                           .build();

        const std::vector<std::pair<std::string, std::string>> buttons = {
            {"solo", "Solo"},
            {"multi", "Multiplayer"},
            {"settings", "Settings"},
            {"quit", "Quit"}};

        const float spacing = 55.F;
        const float totalHeight = spacing * static_cast<float>(buttons.size() - 1);
        float startY = (H / 2.F) - (totalHeight / 2.F) + 60.F;

        for (const auto &[id, label] : buttons)
        {
            ecs::Entity e = reg.createEntity()
                                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                                .with<ecs::Transform>("transform_" + id, W / 2.F - 80.F, startY, 0.F)
                                .with<ecs::Color>("color_" + id, GRAY.r, GRAY.g, GRAY.b, 255)
                                .with<ecs::Text>(id, label, 32U)
                                .build();
            m_items.push_back({e, id});
            startY += spacing;
        }

        for (int i = 0; i < 160; i++)
        {
            float x = static_cast<float>(std::rand() % static_cast<int>(W));
            float y = static_cast<float>(std::rand() % static_cast<int>(H));
            float vel = -20.F - static_cast<float>(std::rand() % 50);
            int color = 150 + (std::rand() % 105);
            reg.createEntity()
                .with<ecs::Pixel>("star_" + std::to_string(i))
                .with<ecs::Transform>("tr_star_" + std::to_string(i), x, y, 0.F)
                .with<ecs::Velocity>("vel_star_" + std::to_string(i), vel, 0.F)
                .with<ecs::Color>("col_star_" + std::to_string(i), color, color, 255, 255)
                .build();
        }

        updateHighlight();
        std::cout << "[Menu] Entities created (logo + centered buttons + stars)\n";
    }

    void Menu::updateHighlight()
    {
        auto &reg = getRegistry();
        for (std::size_t i = 0; i < m_items.size(); ++i)
        {
            auto *color = reg.getComponent<ecs::Color>(m_items[i].entity);
            auto *text = reg.getComponent<ecs::Text>(m_items[i].entity);
            if (!color || !text)
                continue;

            if (static_cast<int>(i) == m_selectedIndex)
            {
                color->r = CYAN_GLOW.r;
                color->g = CYAN_GLOW.g;
                color->b = CYAN_GLOW.b;
                text->font_size = 44U;
            }
            else
            {
                color->r = GRAY.r;
                color->g = GRAY.g;
                color->b = GRAY.b;
                text->font_size = 32U;
            }
        }
    }

    void Menu::event(const eng::Event &event)
    {
        if (event.type != eng::EventType::KeyPressed)
            return;

        if (event.key == eng::Key::Up)
        {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_items.size())) % static_cast<int>(m_items.size());
            updateHighlight();
        }
        else if (event.key == eng::Key::Down)
        {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_items.size());
            updateHighlight();
        }
        else if (event.key == eng::Key::Enter)
        {
            const auto &id = m_items[m_selectedIndex].id;

            if (id == "solo")
            {
                std::cout << "[Menu] Starting solo game...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(2);
            }
            else if (id == "multi")
            {
                std::cout << "[Menu] Switching to multiplayer lobby...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(3);
            }
            else if (id == "settings")
            {
                std::cout << "[Menu] Opening settings...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(4);
            }
            else if (id == "quit")
            {
                std::cout << "[Menu] Quitting game...\n";
                std::exit(0);
            }
        }
    }

    void Menu::update(const float dt, const eng::WindowSize &size)
    {
        auto &reg = getRegistry();

        for (auto &[entity, velocity] : reg.getAll<ecs::Velocity>())
        {
            if (auto *transform = reg.getComponent<ecs::Transform>(entity))
            {
                transform->x += velocity.x * dt;
                if (transform->x < 0)
                {
                    transform->x = static_cast<float>(size.width);
                    transform->y = static_cast<float>(std::rand() % static_cast<int>(size.height));
                }
            }
        }

        static float t = 0.F;
        t += dt * 2.0F;
        float pulse = (std::sin(t) + 1.F) * 0.08F + 0.92F;
        float offsetX = std::sin(t * 0.6F) * 10.F;

        auto *scale = reg.getComponent<ecs::Scale>(m_logoEntity);
        auto *transform = reg.getComponent<ecs::Transform>(m_logoEntity);
        if (scale && transform)
        {
            scale->x = 0.8F * pulse;
            scale->y = 0.8F * pulse;
            transform->x = (size.width / 2.F - 128.F) + offsetX;
        }
    }
} // namespace cli
