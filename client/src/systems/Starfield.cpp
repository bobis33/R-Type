#include "Client/Systems/Starfield.hpp"

static constexpr eng::Color WHITE_TRANS = {.r = 255U, .g = 255U, .b = 255U, .a = 100U};
static constexpr eng::Color BLUE = {.r = 200U, .g = 200U, .b = 255U, .a = 150U};
static constexpr eng::Color BLUE_SECOND = {.r = 50U, .g = 100U, .b = 200U, .a = 60U};
static constexpr eng::Color YELLOW = {.r = 255U, .g = 255U, .b = 200U, .a = 200U};
static constexpr eng::Color PURPLE = {.r = 100U, .g = 50U, .b = 150U, .a = 80U};
static constexpr eng::Color GREEN = {.r = 200U, .g = 255U, .b = 200U, .a = 180U};

void cli::StarfieldSystem::createStarfield(ecs::Registry &registry, int screenWidth, int screenHeight)
{
    createStars(registry, 50, screenWidth, screenHeight, WHITE_TRANS, -20.0f, "star_far");
    createStars(registry, 30, screenWidth, screenHeight, BLUE, -40.0f, "star_mid");
    createStars(registry, 20, screenWidth, screenHeight, YELLOW, -80.0f, "star_near");
    createShootingStars(registry, 10, screenWidth, screenHeight);
    createPlanets(registry, 5, screenWidth, screenHeight);
    createNebulae(registry, 3, screenWidth, screenHeight);
    createComets(registry, 8, screenWidth, screenHeight);
}

void cli::StarfieldSystem::createStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight, 
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

void cli::StarfieldSystem::createShootingStars(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("star_shooting")
            .with<ecs::Transform>("star_shooting_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_shooting_color", GREEN.r, GREEN.g, GREEN.b, GREEN.a)
            .with<ecs::Velocity>("star_shooting_vel", -120.0f, static_cast<float>((std::rand() % 20) - 10))
            .build();
    }
}

void cli::StarfieldSystem::createPlanets(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("planet_far")
            .with<ecs::Transform>("planet_far_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("planet_far_color", PURPLE.r, PURPLE.g, PURPLE.b, PURPLE.a)
            .with<ecs::Velocity>("planet_far_vel", -5.0f, 0.0f)
            .build();
    }
}

void cli::StarfieldSystem::createNebulae(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("nebula")
            .with<ecs::Transform>("nebula_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("nebula_color", BLUE_SECOND.r, BLUE_SECOND.g, BLUE_SECOND.b, BLUE_SECOND.a)
            .with<ecs::Velocity>("nebula_vel", -8.0f, 0.0f)
            .build();
    }
}

void cli::StarfieldSystem::createComets(ecs::Registry &registry, int count, int screenWidth, int screenHeight)
{
    for (int i = 0; i < count; ++i)
    {
        registry.createEntity()
            .with<ecs::Pixel>("comet")
            .with<ecs::Transform>("comet_transform", static_cast<float>(std::rand() % screenWidth),
                                  static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("comet_color", GREEN.r, GREEN.g, GREEN.b, GREEN.a)
            .with<ecs::Velocity>("comet_vel", -60.0f, static_cast<float>((std::rand() % 40) - 20))
            .build();
    }
}

void cli::StarfieldSystem::update(ecs::Registry &registry, float dt)
{
    for (auto &[entity, pixel] : registry.getAll<ecs::Pixel>())
    {
        if (auto *transform = registry.getComponent<ecs::Transform>(entity))
        {
            if (auto *velocity = registry.getComponent<ecs::Velocity>(entity))
            {
                transform->x += velocity->x * dt;
                transform->y += velocity->y * dt;

                if (transform->x < -10.0f || transform->x > 970.0f || transform->y < -10.0f || transform->y > 550.0f)
                {
                    transform->x = static_cast<float>(970 + std::rand() % 200);
                    transform->y = static_cast<float>(std::rand() % 540);
                }
            }
        }
    }
}
