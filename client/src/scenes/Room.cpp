#include "Client/Scenes/Room.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include <iostream>

static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

namespace cli
{
    Room::Room(const std::unique_ptr<eng::IRenderer> &renderer,
               const std::unique_ptr<eng::IAudio> &audio,
               bool isHost)
        : m_isHost(isHost)
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

        createRoomEntities();
    }

    void Room::createRoomEntities()
    {
        auto &registry = getRegistry();

        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("room_title", 180.F, 50.F, 0.F)
            .with<ecs::Color>("color_title", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("room_title_text", "Room #1234", 50U)
            .build();
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("slot1", 200.F, 150.F, 0.F)
            .with<ecs::Color>("slot1_color", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("slot1_text", "Player 1: You", 30U)
            .build();

        for (int i = 2; i <= 4; i++) {
            registry.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("slot" + std::to_string(i), 200.F, 150.F + (i - 1) * 40.F, 0.F)
                .with<ecs::Color>("slot" + std::to_string(i) + "_color", 200, 200, 200, 255)
                .with<ecs::Text>("slot" + std::to_string(i) + "_text", "Player " + std::to_string(i) + ": ---", 30U)
                .build();
        }
        if (m_isHost) {
            auto startEntity = registry.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("start_btn", 220.F, 350.F, 0.F)
                .with<ecs::Color>("start_btn_color", 200, 200, 200, 255)
                .with<ecs::Text>("start_btn_text", "Start Game", 30U)
                .build();
            m_items.push_back({startEntity, "start"});
        }
        auto backEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("back_btn", 220.F, 400.F, 0.F)
            .with<ecs::Color>("back_btn_color", 200, 200, 200, 255)
            .with<ecs::Text>("back_btn_text", "Back to Menu", 30U)
            .build();
        m_items.push_back({backEntity, "back"});

        updateHighlight();
    }

    void Room::updateHighlight()
    {
        auto &registry = getRegistry();
        for (int i = 0; i < (int)m_items.size(); i++) {
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

    void Room::update(float, const eng::WindowSize &)
    {
        if (m_startGame) std::cout << "Room: starting game..." << std::endl;
        if (m_backToMenu) std::cout << "Room: back to menu..." << std::endl;
    }

    void Room::event(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed) {
            if (event.key == eng::Key::Up) {
                m_selectedIndex = (m_selectedIndex - 1 + m_items.size()) % m_items.size();
                updateHighlight();
            } else if (event.key == eng::Key::Down) {
                m_selectedIndex = (m_selectedIndex + 1) % m_items.size();
                updateHighlight();
            } else if (event.key == eng::Key::Enter) {
                if (m_items[m_selectedIndex].id == "start") m_startGame = true;
                if (m_items[m_selectedIndex].id == "back") m_backToMenu = true;
            }
        }
    }
}
