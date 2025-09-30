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

        auto startEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_start", 220.F, 200.F, 0.F)
            .with<ecs::Color>("color_start", 200, 200, 200, 255)
            .with<ecs::Text>("start", "Start Game", 30U)
            .build();
        m_items.push_back({startEntity, "start"});

        auto quitEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_quit", 220.F, 250.F, 0.F)
            .with<ecs::Color>("color_quit", 200, 200, 200, 255)
            .with<ecs::Text>("quit", "Quit", 30U)
            .build();
        m_items.push_back({quitEntity, "quit"});

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
                if (m_items[m_selectedIndex].id == "start")
                    m_startGame = true;
                else if (m_items[m_selectedIndex].id == "quit")
                    m_exitGame = true;
            }
        }
    }

    void Menu::update(float, const eng::WindowSize &)
    {
        if (m_exitGame)
            std::cout << "Exiting game..." << std::endl;
        if (m_startGame)
            std::cout << "Starting game..." << std::endl;
    }
}
