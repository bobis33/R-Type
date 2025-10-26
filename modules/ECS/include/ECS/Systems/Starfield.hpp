///
/// @file Starfield.hpp
/// @brief Starfield System for managing background stars and visual effects
/// @namespace ecs
///

#pragma once

#include <ranges>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Common.hpp"

namespace ecs
{

    class StarfieldSystem final : public ASystem
    {
        public:
            explicit StarfieldSystem(const std::shared_ptr<eng::IRenderer> &renderer, Registry &registry) : m_renderer(renderer)
            {
                createStarfield(registry, m_renderer->getWindowSize());
            }
            ~StarfieldSystem() override = default;

            StarfieldSystem(const StarfieldSystem &) = delete;
            StarfieldSystem &operator=(const StarfieldSystem &) = delete;
            StarfieldSystem(StarfieldSystem &&) = delete;
            StarfieldSystem &operator=(StarfieldSystem &&) = delete;

            void update(Registry &registry, const float dt) override
            {
                auto [width, height] = m_renderer->getWindowSize();
                const float screenWidth = static_cast<float>(width);
                const float screenHeight = static_cast<float>(height);

                for (const auto &entity : registry.getAll<Pixel>() | std::views::keys)
                {
                    if (auto *transform = registry.getComponent<Transform>(entity))
                    {
                        if (const auto *velocity = registry.getComponent<Velocity>(entity))
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
                        if (const auto *color = registry.getComponent<Color>(entity))
                        {
                            m_renderer->drawPoint(transform->x, transform->y,
                        {.r = color->r, .g = color->g, .b = color->b, .a = color->a});
                        }
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;

            static void createStarfield(Registry &registry, const eng::WindowSize &windowSize)
            {
                createStars(registry, 50, windowSize.width, windowSize.height, utl::Config::Color::WHITE_TRANS, -20.0f, "star_far");
                createStars(registry, 30, windowSize.width, windowSize.height, utl::Config::Color::BLUE, -40.0f, "star_mid");
                createStars(registry, 20, windowSize.width, windowSize.height, utl::Config::Color::YELLOW, -80.0f, "star_near");
                createShootingStars(registry, 10, windowSize.width, windowSize.height);
                createPlanets(registry, 5, windowSize.width, windowSize.height);
                createNebulae(registry, 3, windowSize.width, windowSize.height);
                createComets(registry, 8, windowSize.width, windowSize.height);
            }
            static void createStars(Registry &registry, const int count, const unsigned int screenWidth, const unsigned int screenHeight,
                             const eng::Color &color, float velocity, const std::string &id)
            {
                for (int i = 0; i < count; ++i)
                {
                    registry.createEntity()
                        .with<Pixel>(id)
                        .with<Transform>(id + "_transform", static_cast<float>(std::rand() % screenWidth),
                                              static_cast<float>(std::rand() % screenHeight), 0.0f)
                        .with<Color>(id + "_color", color.r, color.g, color.b, color.a)
                        .with<Velocity>(id + "_vel", velocity, 0.0f)
                        .build();
                }
            }
            static void createShootingStars(Registry &registry, const int count, const unsigned int screenWidth, const unsigned int screenHeight)
            {
                for (int i = 0; i < count; ++i)
                {
                    registry.createEntity()
                        .with<Pixel>("star_shooting")
                        .with<Transform>("star_shooting_transform", static_cast<float>(std::rand() % screenWidth),
                                              static_cast<float>(std::rand() % screenHeight), 0.0f)
                        .with<Color>("star_shooting_color", utl::Config::Color::GREEN.r, utl::Config::Color::GREEN.g, utl::Config::Color::GREEN.b, utl::Config::Color::GREEN.a)
                        .with<Velocity>("star_shooting_vel", -120.0f, static_cast<float>((std::rand() % 20) - 10))
                        .build();
                }
            }
            static void createPlanets(Registry &registry, const int count, const unsigned int screenWidth, const unsigned int screenHeight)
            {
                for (int i = 0; i < count; ++i)
                {
                    registry.createEntity()
                        .with<Pixel>("planet_far")
                        .with<Transform>("planet_far_transform", static_cast<float>(std::rand() % screenWidth),
                                              static_cast<float>(std::rand() % screenHeight), 0.0f)
                        .with<Color>("planet_far_color", utl::Config::Color::PURPLE.r, utl::Config::Color::PURPLE.g, utl::Config::Color::PURPLE.b, utl::Config::Color::PURPLE.a)
                        .with<Velocity>("planet_far_vel", -5.0f, 0.0f)
                        .build();
                }
            }
            static void createNebulae(Registry &registry, const int count, const unsigned int screenWidth, const unsigned int screenHeight)
            {
                for (int i = 0; i < count; ++i)
                {
                    registry.createEntity()
                        .with<Pixel>("nebula")
                        .with<Transform>("nebula_transform", static_cast<float>(std::rand() % screenWidth),
                                              static_cast<float>(std::rand() % screenHeight), 0.0f)
                        .with<Color>("nebula_color", utl::Config::Color::BLUE_SECOND.r, utl::Config::Color::BLUE_SECOND.g, utl::Config::Color::BLUE_SECOND.b, utl::Config::Color::BLUE_SECOND.a)
                        .with<Velocity>("nebula_vel", -8.0f, 0.0f)
                        .build();
                }
            }
            static void createComets(Registry &registry, const int count, const unsigned int screenWidth, const unsigned int screenHeight)
            {
                for (int i = 0; i < count; ++i)
                {
                    registry.createEntity()
                        .with<Pixel>("comet")
                        .with<Transform>("comet_transform", static_cast<float>(std::rand() % screenWidth),
                                              static_cast<float>(std::rand() % screenHeight), 0.0f)
                        .with<Color>("comet_color", utl::Config::Color::GREEN.r, utl::Config::Color::GREEN.g, utl::Config::Color::GREEN.b, utl::Config::Color::GREEN.a)
                        .with<Velocity>("comet_vel", -60.0f, static_cast<float>((std::rand() % 40) - 20))
                        .build();
                }
            }
    }; // class StarfieldSystem
} // namespace ecs
