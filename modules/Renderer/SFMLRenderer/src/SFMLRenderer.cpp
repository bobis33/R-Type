#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "SFMLRenderer/SFMLRenderer.hpp"

#include <iostream>

struct eng::SFMLRenderer::Impl
{
        sf::RenderWindow window;
        std::unordered_map<std::string, sf::Font> fonts;
        std::unordered_map<std::string, sf::Text> texts;
};

eng::SFMLRenderer::SFMLRenderer() : m_impl(std::make_unique<Impl>()) {}

eng::SFMLRenderer::~SFMLRenderer() = default;

void eng::SFMLRenderer::createWindow(const std::string &title, unsigned int height, unsigned int width,
                                     const unsigned int frameLimit, const bool fullscreen)
{
    const sf::VideoMode mode = fullscreen ? sf::VideoMode::getDesktopMode() : sf::VideoMode({width, height});
    m_impl->window.create(mode, title, fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
    m_impl->window.setFramerateLimit(frameLimit);
}

bool eng::SFMLRenderer::windowIsOpen() const { return m_impl->window.isOpen(); }

void eng::SFMLRenderer::closeWindow() { m_impl->window.close(); }

void eng::SFMLRenderer::setFrameLimit(unsigned int frameLimit) { m_impl->window.setFramerateLimit(frameLimit); }

void eng::SFMLRenderer::createFont(Font font)
{
    sf::Font sfFont;
    if (!sfFont.openFromFile(font.path))
    {
        throw std::runtime_error("Failed to load font: " + font.path);
    }
    m_impl->fonts.emplace(font.name, std::move(sfFont));
}

void eng::SFMLRenderer::createText(Text text)
{
    const auto &font = m_impl->fonts.at(text.fontName);
    sf::Text sfText(font);
    sfText.setString(text.content);
    sfText.setCharacterSize(text.size);
    sfText.setPosition({static_cast<float>(text.x), static_cast<float>(text.y)});
    sfText.setFillColor(sf::Color(text.color.r, text.color.g, text.color.b, text.color.a));
    m_impl->texts.emplace(text.name, std::move(sfText));
}

void eng::SFMLRenderer::setTextContent(const std::string &name, const std::string &content)
{
    if (const auto it = m_impl->texts.find(name); it != m_impl->texts.end())
    {
        it->second.setString(content);
    }
    else
    {
        throw std::runtime_error("Text not found: " + name);
    }
}

void eng::SFMLRenderer::setTextPosition(const std::string &name, int x, int y)
{
    if (const auto it = m_impl->texts.find(name); it != m_impl->texts.end())
    {
        it->second.setPosition({static_cast<float>(x), static_cast<float>(y)});
    }
    else
    {
        throw std::runtime_error("Text not found: " + name);
    }
}

void eng::SFMLRenderer::setTextColor(const std::string &name, const Color color)
{
    if (const auto it = m_impl->texts.find(name); it != m_impl->texts.end())
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
    if (const auto it = m_impl->texts.find(name); it != m_impl->texts.end())
    {
        m_impl->window.draw(it->second);
    }
    else
    {
        throw std::runtime_error("Text not found: " + name);
    }
}

void eng::SFMLRenderer::clearWindow(const Color color)
{
    m_impl->window.clear(sf::Color(color.r, color.g, color.b, color.a));
}

void eng::SFMLRenderer::displayWindow() { m_impl->window.display(); }

static eng::Key scancodeToKey(const sf::Keyboard::Scancode sc)
{
    using S = sf::Keyboard::Scancode;
    switch (sc)
    {
        case S::Escape:
            return eng::Key::Escape;
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
            return eng::Key::Unknown;
    }
}

bool eng::SFMLRenderer::pollEvent(Event &event)
{
    if (const auto eventOpt = m_impl->window.pollEvent())
    {
        const auto &e = *eventOpt;

        if (e.is<sf::Event::Closed>())
        {
            event.type = EventType::Closed;
            return true;
        }

        if (const auto *const key = e.getIf<sf::Event::KeyPressed>())
        {
            event.type = EventType::KeyPressed;
            std::cout << "Key pressed: " << std::to_string(static_cast<int>(key->scancode)) << '\n';
            event.key = scancodeToKey(key->scancode);
            return true;
        }

        if (const auto *const key = e.getIf<sf::Event::KeyReleased>())
        {
            event.type = EventType::KeyReleased;
            std::cout << "Key released: " << std::to_string(static_cast<int>(key->scancode)) << '\n';
            event.key = scancodeToKey(key->scancode);
            return true;
        }

        event.type = EventType::None;
        return true;
    }
    return false;
}
