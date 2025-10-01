#include "Client/Scenes/Lobby.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include <iostream>

static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

namespace cli
{
    Lobby::Lobby(const std::unique_ptr<eng::IRenderer> &renderer,
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

        createLobbyEntities();
    }

    void Lobby::createLobbyEntities()
    {
        auto &registry = getRegistry();

        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("lobby_title_transform", 200.F, 50.F, 0.F)
            .with<ecs::Color>("lobby_title_color", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("lobby_title", "Multiplayer Lobby", 50U)
            .build();

        auto createEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("create_room_transform", 220.F, 150.F, 0.F)
            .with<ecs::Color>("create_room_color", 200, 200, 200, 255)
            .with<ecs::Text>("create_room", "Create Room", 30U)
            .build();
        m_rooms.push_back({createEntity, "create"});

        auto joinEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("join_room_transform", 220.F, 200.F, 0.F)
            .with<ecs::Color>("join_room_color", 200, 200, 200, 255)
            .with<ecs::Text>("join_room", "Join Room", 30U)
            .build();
        m_rooms.push_back({joinEntity, "join"});

        auto backEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("back_menu_transform", 220.F, 250.F, 0.F)
            .with<ecs::Color>("back_menu_color", 200, 200, 200, 255)
            .with<ecs::Text>("back_menu", "Return to Menu", 30U)
            .build();
        m_rooms.push_back({backEntity, "back"});

        updateHighlight();

        std::cout << "Lobby entities created!" << std::endl;
    }

    void Lobby::updateHighlight()
    {
        auto &registry = getRegistry();

        for (int i = 0; i < static_cast<int>(m_rooms.size()); i++) {
            auto *color = registry.getComponent<ecs::Color>(m_rooms[i].entity);
            auto *text = registry.getComponent<ecs::Text>(m_rooms[i].entity);
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

    void Lobby::event(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed) {
            if (event.key == eng::Key::Up) {
                m_selectedIndex = (m_selectedIndex - 1 + m_rooms.size()) % m_rooms.size();
                updateHighlight();
            }
            else if (event.key == eng::Key::Down) {
                m_selectedIndex = (m_selectedIndex + 1) % m_rooms.size();
                updateHighlight();
            }
            else if (event.key == eng::Key::Enter) {
                if (m_rooms[m_selectedIndex].id == "create") {
                    m_startRoom = true;
                    std::cout << "Creating new room..." << std::endl;
                }
                else if (m_rooms[m_selectedIndex].id == "join") {
                    std::cout << "Joining existing room..." << std::endl;
                }
                else if (m_rooms[m_selectedIndex].id == "back") {
                    m_returnMenu = true;
                    std::cout << "Returning to menu..." << std::endl;
                }
            }
        }
    }

    void Lobby::update(float, const eng::WindowSize &)
    {
    }
}
