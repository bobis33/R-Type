#include <cmath>

#include "Client/Scenes/Intro.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "Utils/Common.hpp"

cli::Intro::Intro(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                  const std::shared_ptr<eng::IAudio> &audio)
    : AScene(assignedId), m_renderer(renderer), m_audio(audio)
{
    auto &registry = AScene::getRegistry();

    registry.onComponentAdded(
        [&renderer, &registry, &audio](const ecs::Entity e, const std::type_info &type)
        {
            const auto *audioComp = registry.getComponent<ecs::Audio>(e);
            const auto *colorComp = registry.getComponent<ecs::Color>(e);
            const auto *scaleComp = registry.getComponent<ecs::Scale>(e);
            const auto *textureComp = registry.getComponent<ecs::Texture>(e);
            const auto *transform = registry.getComponent<ecs::Transform>(e);
            const auto *rectComp = registry.getComponent<ecs::Rect>(e);

            if (type == typeid(ecs::Texture))
            {
                const float scale_x = scaleComp ? scaleComp->x : 1.F;
                const float scale_y = scaleComp ? scaleComp->y : 1.F;

                renderer->createTexture(textureComp->id, textureComp->path);

                if (transform && textureComp)
                {
                    if (rectComp)
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y, scale_x, scale_y, static_cast<int>(rectComp->pos_x),
                                               static_cast<int>(rectComp->pos_y), rectComp->size_x, rectComp->size_y);
                    }
                    else
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y);
                    }
                }
            }
            else if (type == typeid(ecs::Audio))
            {
                if (audioComp)
                {
                    audio->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                       audioComp->id + std::to_string(e));
                }
            }
        });

    m_soundEntity =
        registry.createEntity().with<ecs::Audio>("game_begin", "assets/audio/goofy.wav", 20.0F, false, true).build();

    auto [width, height] = renderer->getWindowSize();
    m_logoEntity = registry.createEntity()
                       .with<ecs::Transform>("logo_transform", width / 3, height / 5)
                       .with<ecs::Scale>("logo_scale", 1.5F, 1.5F)
                       .with<ecs::Color>("logo_color", 255, 255, 255, 0)
                       .with<ecs::Texture>("logo_text", utl::Path::Icons::ICON_APP)
                       .build();
}

void cli::Intro::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();

    auto *color = reg.getComponent<ecs::Color>(m_logoEntity);
    m_elapsedTime += dt;

    if (!color)
        return;

    if (m_elapsedTime < 2.0f)
    {
        color->a = static_cast<uint8_t>(std::min(255.0f, (m_elapsedTime / 2.0f) * 255.0f));
    }
    else if (m_elapsedTime < 6.0f)
    {
        color->a = 255;
    }
    else if (m_elapsedTime < 8.0f)
    {
        const float t = (m_elapsedTime - 6.0f) / 2.0f;
        color->a = static_cast<uint8_t>(255.0f * (1.0f - t));
    }
    else
    {
        onLeave();
    }
}

void cli::Intro::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
        {

            if (event.key == eng::Key::Up)
            {
            }
            else if (event.key == eng::Key::Down)
            {
            }
            else if (event.key == eng::Key::Enter)
            {
            }
            break;
        }

        case eng::EventType::KeyReleased:
            if (event.key == eng::Key::Up)
            {
            }
            if (event.key == eng::Key::Down)
            {
            }
            if (event.key == eng::Key::Left)
            {
            }
            if (event.key == eng::Key::Right)
            {
            }
            if (event.key == eng::Key::Space)
            {
            }
            break;

        default:
            break;
    }
}
