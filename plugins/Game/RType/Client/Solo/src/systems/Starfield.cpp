#include "RTypeClientSolo/Systems/Starfield.hpp"
#include "ECS/Component.hpp"
#include "Utils/Common.hpp"

#include <ranges>

void gme::StarfieldSystem::createStarfield(ecs::Registry &registry, const eng::WindowSize &windowSize)
{
    createStars(registry, 50, windowSize.width, windowSize.height, utl::Config::Color::WHITE_TRANS, -20.0f, "star_far");
    createStars(registry, 30, windowSize.width, windowSize.height, utl::Config::Color::BLUE, -40.0f, "star_mid");
    createStars(registry, 20, windowSize.width, windowSize.height, utl::Config::Color::YELLOW, -80.0f, "star_near");
    createShootingStars(registry, 10, windowSize.width, windowSize.height);
    createPlanets(registry, 5, windowSize.width, windowSize.height);
    createNebulae(registry, 3, windowSize.width, windowSize.height);
    createComets(registry, 8, windowSize.width, windowSize.height);
}

void gme::StarfieldSystem::createStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight,
                                       const eng::Color &color, float velocity, const std::string &id)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>(id)
            .with<ecs::Transform>(id + "_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>(id + "_color", color.r, color.g, color.b, color.a)
            .with<ecs::Velocity>(id + "_vel", velocity, 0.0f)
            .build();
    }
}

void gme::StarfieldSystem::createShootingStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_shooting")
            .with<ecs::Transform>("star_shooting_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_shooting_color", utl::Config::Color::GREEN.r, utl::Config::Color::GREEN.g, utl::Config::Color::GREEN.b, utl::Config::Color::GREEN.a)
            .with<ecs::Velocity>("star_shooting_vel", -120.0f, static_cast<float>((std::rand() % 20) - 10))
            .build();
    }
}

void gme::StarfieldSystem::createPlanets(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("planet_far")
            .with<ecs::Transform>("planet_far_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("planet_far_color", utl::Config::Color::PURPLE.r, utl::Config::Color::PURPLE.g, utl::Config::Color::PURPLE.b, utl::Config::Color::PURPLE.a)
            .with<ecs::Velocity>("planet_far_vel", -5.0f, 0.0f)
            .build();
    }
}

void gme::StarfieldSystem::createNebulae(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("nebula")
            .with<ecs::Transform>("nebula_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("nebula_color", utl::Config::Color::BLUE_SECOND.r, utl::Config::Color::BLUE_SECOND.g, utl::Config::Color::BLUE_SECOND.b, utl::Config::Color::BLUE_SECOND.a)
            .with<ecs::Velocity>("nebula_vel", -8.0f, 0.0f)
            .build();
    }
}

void gme::StarfieldSystem::createComets(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("comet")
            .with<ecs::Transform>("comet_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("comet_color", utl::Config::Color::GREEN.r, utl::Config::Color::GREEN.g, utl::Config::Color::GREEN.b, utl::Config::Color::GREEN.a)
            .with<ecs::Velocity>("comet_vel", -60.0f, static_cast<float>((std::rand() % 40) - 20))
            .build();
    }
}

void gme::StarfieldSystem::update(ecs::Registry &registry, const float dt)
{
    auto [width, height] = m_renderer->getWindowSize();
    const float screenWidth = static_cast<float>(width);
    const float screenHeight = static_cast<float>(height);

    for (const auto &entity : registry.getAll<ecs::Pixel>() | std::views::keys)
    {
        if (auto *transform = registry.getComponent<ecs::Transform>(entity))
        {
            if (const auto *velocity = registry.getComponent<ecs::Velocity>(entity))
            {
                transform->x += velocity->x * dt;
                transform->y += velocity->y * dt;

                if (transform->x < -10.0f || transform->x > screenWidth + 10.0f || transform->y < -10.0f ||
                    transform->y > screenHeight + 10.0f)
                {
                    transform->x = screenWidth + std::rand() % 200;
                    transform->y = static_cast<float>(std::rand() % static_cast<int>(screenHeight));
                }
            }
        }
    }
}
