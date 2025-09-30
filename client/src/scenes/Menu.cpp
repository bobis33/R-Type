#include "Client/Scenes/Menu.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include <iostream>

static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

namespace cli
{
    Menu::Menu(const std::unique_ptr<eng::IRenderer> &renderer,
               const std::unique_ptr<eng::IAudio> & /*audio*/)
    {
        auto &registry = AScene::getRegistry();

        registry.onComponentAdded([&renderer, &registry](const ecs::Entity e, const std::type_info &type) {
            const auto *colorComp = registry.getComponent<ecs::Color>(e);
            const auto *fontComp = registry.getComponent<ecs::Font>(e);
            const auto *textComp = registry.getComponent<ecs::Text>(e);
            const auto *transform = registry.getComponent<ecs::Transform>(e);

            if (type == typeid(ecs::Text) && textComp && transform && fontComp) {
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
        });
        createMenuEntities();
    }

    void Menu::createMenuEntities()
    {
        auto &registry = getRegistry();

        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_title", 200.F, 50.F, 0.F)
            .with<ecs::Color>("color_title", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("title", "RType", 60U)
            .build();

        auto soloEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_solo", 220.F, 180.F, 0.F)
            .with<ecs::Color>("color_solo", 200, 200, 200, 255)
            .with<ecs::Text>("solo", "Solo", 30U)
            .build();
        m_items.push_back({soloEntity, "solo"});

        auto multiEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_multi", 220.F, 230.F, 0.F)
            .with<ecs::Color>("color_multi", 200, 200, 200, 255)
            .with<ecs::Text>("multi", "Multiplayer", 30U)
            .build();
        m_items.push_back({multiEntity, "multi"});

        auto quitEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_quit", 220.F, 280.F, 0.F)
            .with<ecs::Color>("color_quit", 200, 200, 200, 255)
            .with<ecs::Text>("quit", "Quit", 30U)
            .build();
        m_items.push_back({quitEntity, "quit"});

        for (int i = 0; i < 100; i++) {
            registry.createEntity()
                .with<ecs::Pixel>("star_point_" + std::to_string(i))
                .with<ecs::Transform>("star_point_transform",
                                       static_cast<float>(std::rand() % 800),
                                       static_cast<float>(std::rand() % 600),
                                       0.F)
                .with<ecs::Velocity>("star_vel", -20.F - static_cast<float>(std::rand() % 30), 0.F)
                .with<ecs::Color>("star_color", 200, 200, 255, 255)
                .build();
        }

        updateHighlight();
        std::cout << "Menu entities created!" << std::endl;
    }

    void Menu::updateHighlight()
    {
        auto &registry = getRegistry();

        for (int i = 0; i < static_cast<int>(m_items.size()); i++) {
            auto *color = registry.getComponent<ecs::Color>(m_items[i].entity);
            auto *text = registry.getComponent<ecs::Text>(m_items[i].entity);
            if (!color || !text) continue;

            if (i == m_selectedIndex) {
                color->r = color->g = color->b = 255;
                text->font_size = 40;
            } else {
                color->r = color->g = color->b = 200;
                text->font_size = 30;
            }
        }
    }

    void Menu::event(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed) {
            if (event.key == eng::Key::Up) {
                m_selectedIndex = (m_selectedIndex - 1 + m_items.size()) % m_items.size();
                updateHighlight();
            }
            else if (event.key == eng::Key::Down) {
                m_selectedIndex = (m_selectedIndex + 1) % m_items.size();
                updateHighlight();
            }
            else if (event.key == eng::Key::Enter) {
                if (m_items[m_selectedIndex].id == "solo")
                    m_startSolo = true;
                else if (m_items[m_selectedIndex].id == "multi")
                    m_startMulti = true;
                else if (m_items[m_selectedIndex].id == "quit")
                    m_exitGame = true;
            }
        }
    }

    void Menu::update(float dt, const eng::WindowSize &size)
    {
        auto &reg = getRegistry();

        for (auto &[entity, velocity] : reg.getAll<ecs::Velocity>()) {
            if (auto *pixel = reg.getComponent<ecs::Pixel>(entity)) {
                if (auto *transform = reg.getComponent<ecs::Transform>(entity)) {
                    transform->x += velocity.x * dt;
                    if (transform->x < 0) {
                        transform->x = static_cast<float>(size.width);
                        transform->y = static_cast<float>(std::rand() % size.height);
                    }
                }
            }
        }
        if (m_exitGame)
            std::cout << "Exiting game..." << std::endl;
        if (m_startSolo)
            std::cout << "Starting SOLO game..." << std::endl;
        if (m_startMulti)
            std::cout << "Starting MULTIPLAYER game..." << std::endl;
    }
}
