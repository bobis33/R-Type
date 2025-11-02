#include <cmath>
#include <cstdlib>
#include <random>

#include "Client/Scenes/WinCondition.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "Utils/Common.hpp"

cli::WinCondition::WinCondition(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                                const std::shared_ptr<eng::IAudio> &audio)
    : AScene(assignedId), m_renderer(renderer), m_audio(audio)
{
    auto &registry = AScene::getRegistry();

    registry.onComponentAdded(
        [&renderer, &registry, &audio](const ecs::Entity e, const std::type_info &type)
        {
            const auto *audioComp = registry.getComponent<ecs::Audio>(e);
            const auto *colorComp = registry.getComponent<ecs::Color>(e);
            const auto *fontComp = registry.getComponent<ecs::Font>(e);
            const auto *scaleComp = registry.getComponent<ecs::Scale>(e);
            const auto *textComp = registry.getComponent<ecs::Text>(e);
            const auto *textureComp = registry.getComponent<ecs::Texture>(e);
            const auto *transform = registry.getComponent<ecs::Transform>(e);
            const auto *rectComp = registry.getComponent<ecs::Rect>(e);

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
            else if (type == typeid(ecs::Texture))
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

    auto [width, height] = renderer->getWindowSize();

    m_soundEntity =
        registry.createEntity().with<ecs::Audio>("victory_sound", "assets/audio/coin.mp3", 30.0F, false, true).build();

    m_titleEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("title_transform", width * 0.32F, height * 0.25F, 0.F)
            .with<ecs::Color>("title_color", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g,
                              utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
            .with<ecs::Text>("title_text", std::string("VICTORY!"), 96U)
            .build();

    m_subtitleEntity = registry.createEntity()
                           .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                           .with<ecs::Transform>("subtitle_transform", width * 0.28F, height * 0.45F, 0.F)
                           .with<ecs::Color>("subtitle_color", utl::Config::Color::GREEN.r, utl::Config::Color::GREEN.g,
                                             utl::Config::Color::GREEN.b, utl::Config::Color::GREEN.a)
                           .with<ecs::Text>("subtitle_text", std::string("Mission Accomplished"), 42U)
                           .build();

    m_instructionEntity =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("instruction_transform", width * 0.25F, height * 0.75F, 0.F)
            .with<ecs::Color>("instruction_color", utl::Config::Color::GRAY_BLUE_SUBTLE.r,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.g, utl::Config::Color::GRAY_BLUE_SUBTLE.b,
                              utl::Config::Color::GRAY_BLUE_SUBTLE.a)
            .with<ecs::Text>("instruction_text", std::string("Press ENTER to quit"), 28U)
            .build();

    m_iconEntity = registry.createEntity()
                       .with<ecs::Transform>("icon_transform", width * 0.45F, height * 0.55F)
                       .with<ecs::Scale>("icon_scale", 0.5F, 0.5F)
                       .with<ecs::Color>("icon_color", utl::Config::Color::WHITE_LOW.r, utl::Config::Color::WHITE_LOW.g,
                                         utl::Config::Color::WHITE_LOW.b, utl::Config::Color::WHITE_LOW.a)
                       .with<ecs::Texture>("icon_texture", utl::Path::Icons::ICON_APP)
                       .build();
}

void cli::WinCondition::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();
    m_elapsedTime += dt;
    m_titlePulseTime += dt;
    m_particleSpawnTimer += dt;

    if (auto *titleColor = reg.getComponent<ecs::Color>(m_titleEntity))
    {
        if (m_elapsedTime < 0.5F)
        {
            titleColor->a = static_cast<uint8_t>(std::min(255.0F, (m_elapsedTime / 0.5F) * 255.0F));
        }
        else
        {
            const float pulse = (std::sin(m_titlePulseTime * 3.0F) + 1.0F) * 0.5F;
            titleColor->r = static_cast<uint8_t>(utl::Config::Color::CYAN_ELECTRIC.r * (0.7F + pulse * 0.3F));
            titleColor->g = static_cast<uint8_t>(utl::Config::Color::CYAN_ELECTRIC.g * (0.7F + pulse * 0.3F));
            titleColor->b = static_cast<uint8_t>(utl::Config::Color::CYAN_ELECTRIC.b * (0.9F + pulse * 0.1F));
            titleColor->a = 255;
        }
    }

    if (auto *titleTransform = reg.getComponent<ecs::Transform>(m_titleEntity))
    {
        if (m_elapsedTime > 0.5F)
        {
            titleTransform->y = size.height * 0.25F + std::sin(m_titlePulseTime * 1.5F) * 8.0F;
        }
    }

    if (auto *subtitleColor = reg.getComponent<ecs::Color>(m_subtitleEntity))
    {
        if (m_elapsedTime > 0.8F && m_elapsedTime < 1.5F)
        {
            const float t = (m_elapsedTime - 0.8F) / 0.7F;
            subtitleColor->a = static_cast<uint8_t>(std::min(255.0F, t * 255.0F));
        }
        else if (m_elapsedTime >= 1.5F)
        {
            subtitleColor->a = 255;
        }
    }

    if (auto *iconColor = reg.getComponent<ecs::Color>(m_iconEntity))
    {
        if (m_elapsedTime > 1.0F && m_elapsedTime < 2.0F)
        {
            const float t = (m_elapsedTime - 1.0F);
            iconColor->a = static_cast<uint8_t>(std::min(255.0F, t * 255.0F));

            if (auto *iconScale = reg.getComponent<ecs::Scale>(m_iconEntity))
            {
                const float scale = 0.5F + std::sin(t * 3.14159F) * 0.2F;
                iconScale->x = scale;
                iconScale->y = scale;
            }
        }
        else if (m_elapsedTime >= 2.0F)
        {
            iconColor->a = 255;
            if (auto *iconScale = reg.getComponent<ecs::Scale>(m_iconEntity))
            {
                const float breathe = 0.5F + std::sin(m_titlePulseTime * 0.8F) * 0.1F;
                iconScale->x = breathe;
                iconScale->y = breathe;
            }
        }
    }

    if (auto *instructionColor = reg.getComponent<ecs::Color>(m_instructionEntity))
    {
        if (m_elapsedTime > 2.5F)
        {
            const float blink = (std::sin(m_titlePulseTime * 2.0F) + 1.0F) * 0.5F;
            instructionColor->a = static_cast<uint8_t>(150.0F + blink * 105.0F);
        }
    }

    if (m_elapsedTime > 1.0F && m_particleSpawnTimer > 0.3F && m_particles.size() < 50)
    {
        m_particleSpawnTimer = 0.0F;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0.0F, 6.28318F);
        std::uniform_real_distribution<float> speedDist(50.0F, 150.0F);
        std::uniform_real_distribution<float> lifetimeDist(1.0F, 3.0F);

        const float angle = angleDist(gen);
        const float speed = speedDist(gen);
        const float lifetime = lifetimeDist(gen);

        Particle p;
        p.x = size.width * 0.5F;
        p.y = size.height * 0.35F;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.lifetime = 0.0F;
        p.maxLifetime = lifetime;

        p.entity = reg.createEntity()
                       .with<ecs::Transform>("particle_transform", p.x, p.y)
                       .with<ecs::Scale>("particle_scale", 0.15F, 0.15F)
                       .with<ecs::Color>("particle_color", utl::Config::Color::CYAN_ELECTRIC.r,
                                         utl::Config::Color::CYAN_ELECTRIC.g, utl::Config::Color::CYAN_ELECTRIC.b,
                                         utl::Config::Color::CYAN_ELECTRIC.a)
                       .with<ecs::Texture>("particle_texture", utl::Path::Icons::ICON_APP)
                       .build();

        m_particles.push_back(p);
    }

    for (auto it = m_particles.begin(); it != m_particles.end();)
    {
        it->lifetime += dt;
        it->x += it->vx * dt;
        it->y += it->vy * dt;

        if (auto *transform = reg.getComponent<ecs::Transform>(it->entity))
        {
            transform->x = it->x;
            transform->y = it->y;
        }

        if (auto *color = reg.getComponent<ecs::Color>(it->entity))
        {
            const float alpha = 1.0F - (it->lifetime / it->maxLifetime);
            color->a = static_cast<uint8_t>(std::max(0.0F, alpha * 255.0F));
        }

        if (it->lifetime >= it->maxLifetime)
        {
            it = m_particles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void cli::WinCondition::event(const eng::Event &event)
{
    if (event.type == eng::EventType::KeyPressed)
    {
        if (event.key == eng::Key::Enter && m_elapsedTime > 2.5F)
        {
            std::exit(0);
        }
    }
}
