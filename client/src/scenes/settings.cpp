#include "Client/Scenes/Settings.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

static constexpr eng::Color CYAN_GLOW = {0U, 200U, 255U, 255U};
static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};
static constexpr eng::Color GRAY = {180U, 180U, 180U, 255U};

namespace cli
{
    Settings::Settings(const std::shared_ptr<eng::IRenderer> &renderer,
                       const std::shared_ptr<eng::IAudio> &audio,
                       eng::SceneManager *sceneManager)
        : m_renderer(*renderer), m_audio(*audio), m_sceneManager(sceneManager)
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
        createSettingsEntities();
    }

    void Settings::createSettingsEntities()
    {
        auto &reg = getRegistry();

        constexpr float windowWidth = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_WIDTH);
        constexpr float windowHeight = static_cast<float>(cli::Config::Window::DEFAULT_WINDOW_HEIGHT);

        m_titleEntity = reg.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("title_transform", windowWidth / 2.F - 140.F, 60.F, 0.F)
            .with<ecs::Color>("title_color", CYAN_GLOW.r, CYAN_GLOW.g, CYAN_GLOW.b, CYAN_GLOW.a)
            .with<ecs::Text>("title_text", "SETTINGS", 50U)
            .build();

        int volume = 50;
        bool fullscreen = false;
        int fps = 240;
        if (m_sceneManager && m_sceneManager->getSettingsManager()) {
            volume = m_sceneManager->getSettingsManager()->getVolume();
            fullscreen = m_sceneManager->getSettingsManager()->isFullscreen();
            fps = m_sceneManager->getSettingsManager()->getFps();
        }
        const std::vector<std::pair<std::string, std::string>> settings = {
            {"volume", "Volume: " + std::to_string(volume)},
            {"fullscreen", std::string("Fullscreen: ") + (fullscreen ? "On" : "Off")},
            {"fps", "Framerate: " + std::to_string(fps)},
            {"back", "Return to Menu"}};

        const float spacing = 60.F;
        const float totalHeight = spacing * static_cast<float>(settings.size() - 1);
        float startY = (windowHeight / 2.F) - (totalHeight / 2.F) + 30.F;

        for (const auto &[id, label] : settings)
        {
            ecs::Entity entity = reg.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("transform_" + id, windowWidth / 2.F - 160.F, startY, 0.F)
                .with<ecs::Color>("color_" + id, GRAY.r, GRAY.g, GRAY.b, 255)
                .with<ecs::Text>(id, label, 32U)
                .build();

            m_items.push_back({entity, id});
            startY += spacing;
        }
        updateHighlight();
        std::cout << "[Settings] Entities created (centered layout)\n";
    }

    void Settings::updateHighlight()
    {
        auto &reg = getRegistry();

        for (std::size_t i = 0; i < m_items.size(); ++i)
        {
            auto *color = reg.getComponent<ecs::Color>(m_items[i].entity);
            auto *text = reg.getComponent<ecs::Text>(m_items[i].entity);
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

    void Settings::event(const eng::Event &event)
    {
        if (event.type != eng::EventType::KeyPressed)
            return;

        if (event.key == eng::Key::Up)
        {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_items.size())) % static_cast<int>(m_items.size());
        }
        else if (event.key == eng::Key::Down)
        {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_items.size());
        }
        else if (event.key == eng::Key::Enter)
        {
            const std::string &id = m_items[static_cast<std::size_t>(m_selectedIndex)].id;
            auto settings = m_sceneManager ? m_sceneManager->getSettingsManager() : nullptr;

            if (id == "volume" && settings)
            {
                int newVolume = (settings->getVolume() + 10) % 110;
                settings->setVolume(newVolume);
                auto *text = getRegistry().getComponent<ecs::Text>(m_items[m_selectedIndex].entity);
                if (text)
                    text->content = "Volume: " + std::to_string(newVolume);
                m_audio.setVolume("title_music", static_cast<float>(newVolume));
            }
            else if (id == "fullscreen" && settings)
            {
                bool newFullscreen = !settings->isFullscreen();
                settings->setFullscreen(newFullscreen);
                auto *text = getRegistry().getComponent<ecs::Text>(m_items[m_selectedIndex].entity);
                if (text)
                    text->content = std::string("Fullscreen: ") + (newFullscreen ? "On" : "Off");
                // Ici, il faudrait appeler une méthode sur le renderer pour appliquer le fullscreen
            }
            else if (id == "fps" && settings)
            {
                int newFps = (settings->getFps() == 60 ? 240 : 60);
                settings->setFps(newFps);
                auto *text = getRegistry().getComponent<ecs::Text>(m_items[m_selectedIndex].entity);
                if (text)
                    text->content = "Framerate: " + std::to_string(newFps);
                m_renderer.setFrameLimit(static_cast<unsigned int>(newFps));
            }
            else if (id == "back")
            {
                std::cout << "[Settings] Returning to MENU...\n";
                if (m_sceneManager)
                    m_sceneManager->switchToScene(1);
            }
        }

        updateHighlight();
    }

    void Settings::update(float dt, const eng::WindowSize &)
    {
        auto &reg = getRegistry();
        static float t = 0.F;
        t += dt * 2.0F;

        auto *color = reg.getComponent<ecs::Color>(m_titleEntity);
        if (color)
        {
            color->r = static_cast<unsigned char>((std::sin(t) + 1.F) * 120.F);
            color->g = static_cast<unsigned char>(200 + std::sin(t) * 30.F);
            color->b = 255;
        }
    }
} // namespace cli
