#include "Client/Scenes/Game.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"

#include <iostream>

static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

namespace cli
{
    Game::Game(const std::unique_ptr<eng::IRenderer> &renderer,
               const std::unique_ptr<eng::IAudio> & /*audio*/)
        : m_renderer(*renderer)
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

        registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("title_transform", 10.F, 10.F, 0.F)
            .with<ecs::Color>("title_color", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("title_text", "RType Game", 50U)
            .build();

        m_fpsEntity = registry.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("fps_transform", 10.F, 70.F, 0.F)
            .with<ecs::Color>("fps_color", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("fps_text", "FPS 0", 20U)
            .build();

        std::cout << "Game entities created!" << std::endl;
    }

    void Game::update(float dt, const eng::WindowSize & /*size*/)
    {
        auto &registry = getRegistry();
        if (auto *fpsText = registry.getComponent<ecs::Text>(m_fpsEntity)) {
            fpsText->content = "FPS " + std::to_string(static_cast<int>(1.f / dt));
        }
    }

    void Game::event(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed && event.key == eng::Key::Escape) {
            std::cout << "Exiting game..." << std::endl;
            m_renderer.closeWindow();
        }
    }
}
