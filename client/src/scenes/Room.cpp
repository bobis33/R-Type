#include "Client/Scenes/Room.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include <iostream>
#include <cmath>

static constexpr eng::Color CYAN_GLOW = {0U, 200U, 255U, 255U};
static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};
static constexpr eng::Color GRAY = {180U, 180U, 180U, 255U};

namespace cli
{
    Room::Room(const std::shared_ptr<eng::IRenderer> &renderer,
               const std::shared_ptr<eng::IAudio> & /*audio*/,
               eng::SceneManager *sceneManager,
               bool isHost)
        : m_isHost(isHost), m_sceneManager(sceneManager)
    {
        auto &registry = AScene::getRegistry();

        registry.onComponentAdded(
            [&renderer, &registry](const ecs::Entity e, const std::type_info &type)
            {
                const auto *color = registry.getComponent<ecs::Color>(e);
                const auto *font = registry.getComponent<ecs::Font>(e);
                const auto *text = registry.getComponent<ecs::Text>(e);
                const auto *transform = registry.getComponent<ecs::Transform>(e);

                if (type == typeid(ecs::Text) && text && transform && font)
                {
                    renderer->createFont(font->id, font->path);
                    renderer->createText({
                        .font_name = font->id,
                        .color = {.r = color->r, .g = color->g, .b = color->b, .a = color->a},
                        .content = text->content,
                        .size = text->font_size,
                        .x = transform->x,
                        .y = transform->y,
                        .name = text->id});
                }
            });

        createRoomEntities();
    }

    void Room::createRoomEntities()
    {
        auto &registry = getRegistry();

        constexpr float windowWidth = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_WIDTH);
        constexpr float windowHeight = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_HEIGHT);

        m_titleEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("room_title_transform", windowWidth / 2.F - 180.F, 60.F, 0.F)
            .with<ecs::Color>("room_title_color", CYAN_GLOW.r, CYAN_GLOW.g, CYAN_GLOW.b, CYAN_GLOW.a)
            .with<ecs::Text>("room_title", "ROOM #1234", 50U)
            .build();

        float startY = 160.F;
        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("slot1_transform", windowWidth / 2.F - 150.F, startY, 0.F)
            .with<ecs::Color>("slot1_color", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("slot1_text", "Player 1: You", 30U)
            .build();

        for (int i = 2; i <= 4; ++i)
        {
            startY += 40.F;
            registry.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("slot" + std::to_string(i) + "_transform", windowWidth / 2.F - 150.F, startY, 0.F)
                .with<ecs::Color>("slot" + std::to_string(i) + "_color", 200, 200, 200, 255)
                .with<ecs::Text>("slot" + std::to_string(i) + "_text", "Player " + std::to_string(i) + ": ---", 30U)
                .build();
        }

        float buttonY = startY + 80.F;
        if (m_isHost)
        {
            ecs::Entity startEntity = registry.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("start_btn_transform", windowWidth / 2.F - 80.F, buttonY, 0.F)
                .with<ecs::Color>("start_btn_color", GRAY.r, GRAY.g, GRAY.b, 255)
                .with<ecs::Text>("start_btn_text", "Start Game", 32U)
                .build();
            m_items.push_back({startEntity, "start"});
            buttonY += 50.F;
        }

        ecs::Entity backEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("back_btn_transform", windowWidth / 2.F - 90.F, buttonY, 0.F)
            .with<ecs::Color>("back_btn_color", GRAY.r, GRAY.g, GRAY.b, 255)
            .with<ecs::Text>("back_btn_text", "Back to Menu", 32U)
            .build();
        m_items.push_back({backEntity, "back"});

        updateHighlight();
        std::cout << "[Room] Entities created\n";
    }

    void Room::updateHighlight()
    {
        auto &registry = getRegistry();

        for (std::size_t i = 0; i < m_items.size(); ++i)
        {
            auto *color = registry.getComponent<ecs::Color>(m_items[i].entity);
            auto *text = registry.getComponent<ecs::Text>(m_items[i].entity);
            if (!color || !text)
                continue;

            if (static_cast<int>(i) == m_selectedIndex)
            {
                color->r = CYAN_GLOW.r;
                color->g = CYAN_GLOW.g;
                color->b = CYAN_GLOW.b;
                text->font_size = 40;
            }
            else
            {
                color->r = GRAY.r;
                color->g = GRAY.g;
                color->b = GRAY.b;
                text->font_size = 32;
            }
        }
    }

    void Room::event(const eng::Event &event)
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
            if (id == "start")
            {
                std::cout << "[Room] Host starting the game...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(2);
            }
            else if (id == "back")
            {
                std::cout << "[Room] Returning to Menu...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(1);
            }
        }
    }

    void Room::update(float dt, const eng::WindowSize &)
    {
        auto &reg = getRegistry();
        static float t = 0.F;
        t += dt * 2.0F;

        auto *color = reg.getComponent<ecs::Color>(m_titleEntity);
        if (color)
        {
            color->r = static_cast<unsigned char>((std::sin(t) + 1.F) * 100.F);
            color->g = static_cast<unsigned char>(200 + std::sin(t) * 30.F);
            color->b = 255;
        }
    }
} // namespace cli
