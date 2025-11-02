#include "RTypeClientMulti/Scenes/GameOver.hpp"
#include "ECS/Component.hpp"
#include "Utils/Common.hpp"
#include <cmath>

static constexpr eng::Color RED_COLOR = {.r = 255U, .g = 50U, .b = 50U, .a = 255U};
static constexpr eng::Color WHITE_COLOR = {.r = 255U, .g = 255U, .b = 255U, .a = 255U};

namespace gme
{
    GameOverScene::GameOverScene(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer)
        : AScene(assignedId), m_renderer(renderer)
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

    void GameOverScene::update(float dt, const eng::WindowSize & /*size*/)
    {
        m_animationTime += dt;

        // Add pulsing animation to "GAME OVER" text
        auto &registry = AScene::getRegistry();

        // Find and animate the game over text
        auto textEntities = registry.getAll<ecs::Text>();

        for (const auto &[entity, text] : textEntities)
        {
            auto *color = registry.getComponent<ecs::Color>(entity);

            if (color && text.content == "GAME OVER")
            {
                // Pulsing effect: oscillate between 150 and 255 alpha
                float pulse = (std::sin(m_animationTime * 3.0f) + 1.0f) / 2.0f; // 0.0 to 1.0
                color->a = static_cast<unsigned char>(150 + pulse * 105);
            }
        }
    }

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
