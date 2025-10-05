#include "Client/Scenes/Settings.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include <iostream>
#include <algorithm>

static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

namespace cli
{
    Settings::Settings(const std::unique_ptr<eng::IRenderer> &renderer,
                       const std::unique_ptr<eng::IAudio> &audio)
        : m_renderer(*renderer), m_audio(*audio)
    {
        auto &registry = AScene::getRegistry();

        registry.onComponentAdded(
            [&renderer, &registry](const ecs::Entity e, const std::type_info &type)
            {
                const auto *color = registry.getComponent<ecs::Color>(e);
                const auto *font = registry.getComponent<ecs::Font>(e);
                const auto *text = registry.getComponent<ecs::Text>(e);
                const auto *transform = registry.getComponent<ecs::Transform>(e);

                if (type == typeid(ecs::Text) && text && transform && font) {
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

        reg.createEntity()
            .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("title", 180.F, 50.F, 0.F)
            .with<ecs::Color>("title_color", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
            .with<ecs::Text>("title_text", "Settings", 50U)
            .build();

        const std::vector<std::pair<std::string, std::string>> settings = {
            {"volume", "Volume: 50"},
            {"fullscreen", "Fullscreen: Off"},
            {"fps", "Framerate: 240"},
            {"back", "Return to Menu"}};

        float y = 180.F;
        for (const auto &[id, label] : settings) {
            ecs::Entity entity = reg.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("tr_" + id, 220.F, y, 0.F)
                .with<ecs::Color>("color_" + id, 200, 200, 200, 255)
                .with<ecs::Text>(id, label, 30U)
                .build();

            m_items.push_back({entity, id});
            y += 50.F;
        }
        updateHighlight();
        std::cout << "Settings entities created!" << std::endl;
    }

    void Settings::updateHighlight()
    {
        auto &reg = getRegistry();

        for (std::size_t i = 0; i < m_items.size(); ++i) {
            auto *color = reg.getComponent<ecs::Color>(m_items[i].entity);
            auto *text = reg.getComponent<ecs::Text>(m_items[i].entity);
            if (!color || !text)
                continue;
            if (static_cast<int>(i) == m_selectedIndex) {
                color->r = 100;
                color->g = 255;
                color->b = 255;
                text->font_size = 40;
            } else {
                color->r = color->g = color->b = 200;
                text->font_size = 30;
            }
        }
    }

    void Settings::event(const eng::Event &event)
    {
        if (event.type == eng::EventType::KeyPressed) {
            if (event.key == eng::Key::Up) {
                m_selectedIndex = static_cast<int>((m_selectedIndex - 1 + static_cast<int>(m_items.size())) % static_cast<int>(m_items.size()));
            } else if (event.key == eng::Key::Down) {
                m_selectedIndex = static_cast<int>((m_selectedIndex + 1) % static_cast<int>(m_items.size()));
            } else if (event.key == eng::Key::Enter) {
                const std::string &id = m_items[static_cast<std::size_t>(m_selectedIndex)].id;

                if (id == "volume") {
                    m_volume = (m_volume + 10) % 110;
                    auto *text = getRegistry().getComponent<ecs::Text>(m_items[static_cast<std::size_t>(m_selectedIndex)].entity);
                    if (text)
                        text->content = "Volume: " + std::to_string(m_volume);
                    m_audio.setVolume("title_music", static_cast<float>(m_volume));
                } else if (id == "fullscreen") {
                    m_fullscreen = !m_fullscreen;
                    auto *text = getRegistry().getComponent<ecs::Text>(m_items[static_cast<std::size_t>(m_selectedIndex)].entity);
                    if (text)
                        text->content = std::string("Fullscreen: ") + (m_fullscreen ? "On" : "Off");
                } else if (id == "fps") {
                    m_fps = (m_fps == 60 ? 240 : 60);
                    auto *text = getRegistry().getComponent<ecs::Text>(m_items[static_cast<std::size_t>(m_selectedIndex)].entity);
                    if (text)
                        text->content = "Framerate: " + std::to_string(m_fps);
                } else if (id == "back") {
                    m_returnMenu = true;
                }
            }
            updateHighlight();
        }
    }

    void Settings::update(float, const eng::WindowSize &)
    {
    }
}
