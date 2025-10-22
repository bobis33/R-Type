#include <iostream>

#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics.hpp>

#include "SFMLRenderer/SFMLRenderer.hpp"

#include "Utils/Logger.hpp"

eng::SFMLRenderer::~SFMLRenderer() { ImGui::SFML::Shutdown(); }

void eng::SFMLRenderer::createWindow(const std::string &title, unsigned int height, unsigned int width,
                                     const unsigned int frameLimit, const bool fullscreen)
{
    const sf::VideoMode mode = fullscreen ? sf::VideoMode::getDesktopMode() : sf::VideoMode({width, height});
    window.create(mode, title, fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
    window.setFramerateLimit(frameLimit);
    ImGui::SFML::Init(window);
}

bool eng::SFMLRenderer::windowIsOpen() const { return window.isOpen(); }

void eng::SFMLRenderer::closeWindow() { window.close(); }

void eng::SFMLRenderer::setFrameLimit(const unsigned int frameLimit) { window.setFramerateLimit(frameLimit); }

void eng::SFMLRenderer::createFont(const std::string &name, const std::string &path)
{
    sf::Font sfFont;
    if (!sfFont.openFromFile(path))
    {
        throw std::runtime_error("Failed to load font: " + path);
    }
    fonts.emplace(name, std::move(sfFont));
}

void eng::SFMLRenderer::createText(Text text)
{
    const auto &font = fonts.at(text.font_name);
    sf::Text sfText(font);
    sfText.setString(text.content);
    sfText.setCharacterSize(text.size);
    sfText.setPosition({(text.x), text.y});
    sfText.setFillColor(sf::Color(text.color.r, text.color.g, text.color.b, text.color.a));
    texts.emplace(text.name, std::move(sfText));
}

void eng::SFMLRenderer::setTextContent(const std::string &name, const std::string &content)
{
    if (const auto it = texts.find(name); it != texts.end())
    {
        it->second.setString(content);
    }
    else
    {
        throw std::runtime_error("Text not found: " + name);
    }
}

void eng::SFMLRenderer::setTextPosition(const std::string &name, const float x, const float y)
{
    if (const auto it = texts.find(name); it != texts.end())
    {
        it->second.setPosition({x, y});
    }
    else
    {
        throw std::runtime_error("Text not found: " + name);
    }
}

void eng::SFMLRenderer::setTextColor(const std::string &name, const Color color)
{
    if (const auto it = texts.find(name); it != texts.end())
    {
        it->second.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
    }
    else
    {
        throw std::runtime_error("Text not found: " + name);
    }
}

void eng::SFMLRenderer::drawText(const std::string &name)
{
    if (const auto it = texts.find(name); it != texts.end())
    {
        window.draw(it->second);
    }
    else
    {
        throw std::runtime_error("Text not found: " + name);
    }
}

void eng::SFMLRenderer::clearWindow(const Color color) { window.clear(sf::Color(color.r, color.g, color.b, color.a)); }

void eng::SFMLRenderer::displayWindow() { window.display(); }

static eng::Key scancodeToKey(const sf::Keyboard::Scancode sc)
{
    using S = sf::Keyboard::Scancode;
    switch (sc)
    {
        case S::LControl:
            return eng::Key::LControl;
        case S::Escape:
            return eng::Key::Escape;
        case S::Enter:
            return eng::Key::Enter;
        case S::Space:
            return eng::Key::Space;
        case S::Up:
            return eng::Key::Up;
        case S::Down:
            return eng::Key::Down;
        case S::Left:
            return eng::Key::Left;
        case S::Right:
            return eng::Key::Right;
        case S::Delete:
            return eng::Key::Delete;
        case S::A:
            return eng::Key::A;
        case S::B:
            return eng::Key::B;
        case S::C:
            return eng::Key::C;
        case S::D:
            return eng::Key::D;
        case S::E:
            return eng::Key::E;
        case S::F:
            return eng::Key::F;
        case S::G:
            return eng::Key::G;
        case S::H:
            return eng::Key::H;
        case S::I:
            return eng::Key::I;
        case S::J:
            return eng::Key::J;
        case S::K:
            return eng::Key::K;
        case S::L:
            return eng::Key::L;
        case S::M:
            return eng::Key::M;
        case S::N:
            return eng::Key::N;
        case S::O:
            return eng::Key::O;
        case S::P:
            return eng::Key::P;
        case S::Q:
            return eng::Key::Q;
        case S::R:
            return eng::Key::R;
        case S::S:
            return eng::Key::S;
        case S::T:
            return eng::Key::T;
        case S::U:
            return eng::Key::U;
        case S::V:
            return eng::Key::V;
        case S::W:
            return eng::Key::W;
        case S::X:
            return eng::Key::X;
        case S::Y:
            return eng::Key::Y;
        case S::Z:
            return eng::Key::Z;
        case S::Num0:
            return eng::Key::Num0;
        case S::Num1:
            return eng::Key::Num1;
        case S::Num2:
            return eng::Key::Num2;
        case S::Num3:
            return eng::Key::Num3;
        case S::Num4:
            return eng::Key::Num4;
        case S::Num5:
            return eng::Key::Num5;
        case S::Num6:
            return eng::Key::Num6;
        case S::Num7:
            return eng::Key::Num7;
        case S::Num8:
            return eng::Key::Num8;
        case S::Num9:
            return eng::Key::Num9;
        default:
            if (static_cast<int>(sc) == 38 ||   // macOS
                static_cast<int>(sc) == 83 ||   // Windows
                static_cast<int>(sc) == 119)    // Linux
            {
                return eng::Key::Delete;
            }
            return eng::Key::Unknown;
    }
}

bool eng::SFMLRenderer::pollEvent(Event &event)
{
    if (const auto eventOpt = window.pollEvent())
    {
        const auto &e = *eventOpt;
        ImGui::SFML::ProcessEvent(window, e);

        if (e.is<sf::Event::Closed>())
        {
            event.type = EventType::Closed;
            return true;
        }

        if (const auto *const key = e.getIf<sf::Event::KeyPressed>())
        {
            if (key->scancode == sf::Keyboard::Scancode::LControl)
            {
                showDebugOverlay = !showDebugOverlay;
            }
            event.type = EventType::KeyPressed;
            utl::Logger::log("Key pressed: " + std::to_string(static_cast<int>(key->scancode)), utl::LogLevel::INFO);
            event.key = scancodeToKey(key->scancode);
            return true;
        }

        if (const auto *const key = e.getIf<sf::Event::KeyReleased>())
        {
            event.type = EventType::KeyReleased;
            utl::Logger::log("Key pressed: " + std::to_string(static_cast<int>(key->scancode)), utl::LogLevel::INFO);
            event.key = scancodeToKey(key->scancode);
            return true;
        }

        event.type = EventType::None;
        return true;
    }
    return false;
}

void eng::SFMLRenderer::createSprite(const std::string &name, const std::string &textureName, const float x,
                                     const float y, float scale_x, float scale_y, int fx, int fy, int fnx, int fny)
{
    sf::Sprite sfSprite(textures[textureName]);
    sfSprite.setPosition({x, y});
    sfSprite.setScale({scale_x, scale_y});
    if (fnx == -1)
    {
        fnx = static_cast<int>(textures[textureName].getSize().x);
    }
    if (fny == -1)
    {
        fny = static_cast<int>(textures[textureName].getSize().y);
    }
    sfSprite.setTextureRect(sf::IntRect({fx, fy}, {fnx, fny}));

    sprites.emplace(name, std::move(sfSprite));
}

void eng::SFMLRenderer::createTexture(const std::string &name, const std::string &path)
{
    if (textures.contains(name))
    {
        return;
    }

    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        throw std::runtime_error("Failed to load texture: " + path);
    }
    textures[name] = std::move(texture);
}

void eng::SFMLRenderer::drawSprite(const std::string &name)
{
    if (const auto it = sprites.find(name); it != sprites.end())
    {
        window.draw(it->second);
    }
    else
    {
        throw std::runtime_error("Sprite not found: " + name);
    }
}

void eng::SFMLRenderer::setSpritePosition(const std::string &name, const float x, const float y)
{
    if (const auto it = sprites.find(name); it != sprites.end())
    {
        it->second.setPosition({x, y});
    }
    else
    {
        throw std::runtime_error("Sprite not found: " + name);
    }
}

void eng::SFMLRenderer::setSpriteTexture(const std::string &name, const std::string &path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    textures[name] = std::move(texture);

    if (const auto it = sprites.find(name); it != sprites.end())
    {
        it->second.setTexture(textures[name]);
    }
    else
    {
        throw std::runtime_error("Sprite not found: " + name);
    }
}

void eng::SFMLRenderer::setSpriteFrame(const std::string &name, int fx, int fy, int fnx, int fny)
{
    if (const auto it = sprites.find(name); it != sprites.end())
    {
        it->second.setTextureRect(sf::IntRect({fx, fy}, {fnx, fny}));
    }
    else
    {
        throw std::runtime_error("Sprite not found: " + name);
    }
}

void eng::SFMLRenderer::setSpriteScale(const std::string &name, const int x, const int y)
{
    if (const auto it = sprites.find(name); it != sprites.end())
    {
        it->second.setScale({static_cast<float>(x), static_cast<float>(y)});
    }
    else
    {
        throw std::runtime_error("Sprite not found: " + name);
    }
}

void eng::SFMLRenderer::drawPoint(const float x, const float y, const Color color)
{
    const sf::Vertex point(sf::Vector2f(x, y), sf::Color(color.r, color.g, color.b, color.a));
    window.draw(&point, 1, sf::PrimitiveType::Points);
}

eng::WindowSize eng::SFMLRenderer::getWindowSize()
{
    const sf::Vector2u size = window.getSize();
    return {.width = size.x, .height = size.y};
}

void eng::SFMLRenderer::createCircleShape(eng::CircleShape circleShape)
{
    sf::CircleShape sfCircle(circleShape.radius);
    sfCircle.setFillColor(
        sf::Color(circleShape.color.r, circleShape.color.g, circleShape.color.b, circleShape.color.a));
    sfCircle.setPosition({circleShape.x, circleShape.y});
    sfCircle.setOutlineThickness(circleShape.outline_thickness);
    sfCircle.setOutlineColor(sf::Color(circleShape.outline_color.r, circleShape.outline_color.g,
                                       circleShape.outline_color.b, circleShape.outline_color.a));
    circleShapes.emplace(circleShape.name, std::move(sfCircle));
}

void eng::SFMLRenderer::setCircleShapePosition(const std::string &name, float x, float y)
{
    if (const auto it = circleShapes.find(name); it != circleShapes.end())
    {
        it->second.setPosition({x, y});
    }
    else
    {
        throw std::runtime_error("CircleShape not found: " + name);
    }
}
void eng::SFMLRenderer::drawCircleShape(const std::string &name)
{
    if (const auto it = circleShapes.find(name); it != circleShapes.end())
    {
        window.draw(it->second);
    }
    else
    {
        throw std::runtime_error("CircleShape not found: " + name);
    }
}

void eng::SFMLRenderer::renderGui(const WindowSize &windowSize)
{
    ImGui::SFML::Update(window, deltaClock.restart());
    ImGui::SetNextWindowPos(ImVec2(windowSize.width - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.35f);

    constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                              ImGuiWindowFlags_NoNav;
    ImGui::Begin("FPS Overlay", nullptr, window_flags);
    const float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f (%.3f ms/frame)", fps, 1000.0f / fps);
    ImGui::End();
    ImGui::SFML::Render(window);
}