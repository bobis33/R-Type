#include "RTypeClientMulti/Scenes/GameOver.hpp"
#include "ECS/Component.hpp"
#include "Utils/Common.hpp"

static constexpr eng::Color RED_COLOR = {255U, 50U, 50U, 255U};
static constexpr eng::Color WHITE_COLOR = {255U, 255U, 255U, 255U};

namespace gme
{
    GameOverScene::GameOverScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
        : eng::AScene(assignedId), m_renderer(renderer)
    {
        auto &registry = AScene::getRegistry();

        registry.onComponentAdded(
            [&renderer, &registry](const ecs::Entity e, const std::type_info &type)
            {
                const auto *colorComp = registry.getComponent<ecs::Color>(e);
                const auto *fontComp = registry.getComponent<ecs::Font>(e);
                const auto *textComp = registry.getComponent<ecs::Text>(e);
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
            });

        // Game Over title
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_gameover", 150.F, 150.F, 0.F)
            .with<ecs::Color>("color_gameover", RED_COLOR.r, RED_COLOR.g, RED_COLOR.b, RED_COLOR.a)
            .with<ecs::Text>("gameover_text", std::string("GAME OVER"), 72U)
            .build();

        // "Bande de Nuls" text
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_message", 150.F, 260.F, 0.F)
            .with<ecs::Color>("color_message", WHITE_COLOR.r, WHITE_COLOR.g, WHITE_COLOR.b, WHITE_COLOR.a)
            .with<ecs::Text>("message_text", std::string("Bande de Nuls"), 48U)
            .build();

        // Instruction text
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_instruction", 150.F, 350.F, 0.F)
            .with<ecs::Color>("color_instruction", WHITE_COLOR.r, WHITE_COLOR.g, WHITE_COLOR.b, WHITE_COLOR.a)
            .with<ecs::Text>("instruction_text", std::string("Press ESC to return to menu"), 28U)
            .build();
    }

    void GameOverScene::update(float dt, const eng::WindowSize & /*size*/) { m_animationTime += dt; }

    void GameOverScene::event(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed)
        {
            if (event.key == eng::Key::Escape)
            {
                if (onBackToMenu)
                {
                    onBackToMenu();
                }
            }
        }
    }
} // namespace gme
