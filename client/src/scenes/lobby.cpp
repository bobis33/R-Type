#include "Client/Scenes/Lobby.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include <iostream>
#include <cmath>

static constexpr eng::Color CYAN_GLOW = {0U, 200U, 255U, 255U};
static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};
static constexpr eng::Color GRAY = {180U, 180U, 180U, 255U};

namespace cli
{
    Lobby::Lobby(const std::shared_ptr<eng::IRenderer> &renderer,
                 const std::shared_ptr<eng::IAudio> & /*audio*/,
                 eng::SceneManager *sceneManager)
        : m_sceneManager(sceneManager)
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

        createLobbyEntities();
    }

    void Lobby::createLobbyEntities()
    {
        auto &registry = getRegistry();

        constexpr float W = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_WIDTH);
        constexpr float H = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_HEIGHT);

        m_titleEntity = registry.createEntity()
                            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                            .with<ecs::Transform>("lobby_title_transform", W / 2.f - 240.F, 60.F, 0.F)
                            .with<ecs::Color>("lobby_title_color", CYAN_GLOW.r, CYAN_GLOW.g, CYAN_GLOW.b, CYAN_GLOW.a)
                            .with<ecs::Text>("lobby_title", "MULTIPLAYER LOBBY", 48U)
                            .build();

        const std::vector<std::pair<std::string, std::string>> options = {
            {"create", "Create Room"},
            {"join", "Join Room"},
            {"back", "Return to Menu"}};

        const float spacing = 60.F;
        const float totalHeight = spacing * static_cast<float>(options.size() - 1);
        float startY = (H / 2.F) - (totalHeight / 2.F);

        for (const auto &[id, label] : options)
        {
            ecs::Entity e = registry.createEntity()
                                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                                .with<ecs::Transform>("transform_" + id, W / 2.F - 100.F, startY, 0.F)
                                .with<ecs::Color>("color_" + id, GRAY.r, GRAY.g, GRAY.b, 255)
                                .with<ecs::Text>(id, label, 32U)
                                .build();
            m_rooms.push_back({e, id});
            startY += spacing;
        }

        updateHighlight();
        std::cout << "[Lobby] Entities created (centered)\n";
    }

    void Lobby::updateHighlight()
    {
        auto &reg = getRegistry();

        for (std::size_t i = 0; i < m_rooms.size(); ++i)
        {
            auto *color = reg.getComponent<ecs::Color>(m_rooms[i].entity);
            auto *text = reg.getComponent<ecs::Text>(m_rooms[i].entity);
            if (!color || !text)
                continue;

            if (static_cast<int>(i) == m_selectedIndex)
            {
                color->r = CYAN_GLOW.r;
                color->g = CYAN_GLOW.g;
                color->b = CYAN_GLOW.b;
                text->font_size = 40U;
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

    void Lobby::event(const eng::Event &event)
    {
        if (event.type != eng::EventType::KeyPressed)
            return;

        if (event.key == eng::Key::Up)
        {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_rooms.size())) % static_cast<int>(m_rooms.size());
            updateHighlight();
        }
        else if (event.key == eng::Key::Down)
        {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_rooms.size());
            updateHighlight();
        }
        else if (event.key == eng::Key::Enter)
        {
            const auto &id = m_rooms[m_selectedIndex].id;

            if (id == "create")
            {
                std::cout << "[Lobby] Switching to Room (host mode)...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(3);
            }
            else if (id == "join")
            {
                std::cout << "[Lobby] Joining a room (not implemented)\n";
            }
            else if (id == "back")
            {
                std::cout << "[Lobby] Returning to Menu...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(1);
            }
        }
    }

    void Lobby::update(float dt, const eng::WindowSize &)
    {
        auto &reg = getRegistry();
        static float t = 0.F;
        t += dt * 2.5F;

        auto *color = reg.getComponent<ecs::Color>(m_titleEntity);
        if (color)
        {
            color->r = static_cast<unsigned char>((std::sin(t) + 1.F) * 120.F);
            color->g = static_cast<unsigned char>(200 + std::sin(t) * 30.F);
            color->b = 255;
        }
    }
} // namespace cli
