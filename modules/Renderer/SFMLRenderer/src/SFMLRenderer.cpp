#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "SFMLRenderer/SFMLRenderer.hpp"

struct eng::SFMLRenderer::Impl
{
        std::unordered_map<std::string, sf::Texture> textures;

        sf::RenderWindow window;
        std::unordered_map<std::string, sf::Font> fonts;
        std::unordered_map<std::string, sf::Text> texts;
        std::unordered_map<std::string, sf::Sprite> sprites;
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

void eng::SFMLRenderer::setFrameLimit(const unsigned int frameLimit) { m_impl->window.setFramerateLimit(frameLimit); }

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
    sfText.setPosition({(text.x), text.y});
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

void eng::SFMLRenderer::setTextPosition(const std::string &name, const float x, const float y)
{
    if (const auto it = m_impl->texts.find(name); it != m_impl->texts.end())
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

void eng::SFMLRenderer::createSprite(const std::string &path, const float x, const float y, const std::string &name,
                                     float scale_x, float scale_y, int fx, int fy, int fnx, int fny)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    m_impl->textures[name] = std::move(texture);
    sf::Sprite sfSprite(m_impl->textures[name]);
    sfSprite.setPosition({x, y});
    sfSprite.setScale({scale_x, scale_y});
    if (fnx == -1)
    {
        fnx = static_cast<int>(m_impl->textures[name].getSize().x);
    }
    if (fny == -1)
    {
        fny = static_cast<int>(m_impl->textures[name].getSize().y);
    }
    sfSprite.setTextureRect(sf::IntRect({fx, fy}, {fnx, fny}));

    m_impl->sprites.emplace(name, std::move(sfSprite));
}

void eng::SFMLRenderer::createTexture(const std::string &path, const std::string &name)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    m_impl->textures[name] = std::move(texture);
}

void eng::SFMLRenderer::drawSprite(const std::string &name)
{
    if (const auto it = m_impl->sprites.find(name); it != m_impl->sprites.end())
    {
        m_impl->window.draw(it->second);
    }
    else
    {
        throw std::runtime_error("Sprite not found: " + name);
    }
}

void eng::SFMLRenderer::setSpritePosition(const std::string &name, const float x, const float y)
{
    if (const auto it = m_impl->sprites.find(name); it != m_impl->sprites.end())
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

    m_impl->textures[name] = std::move(texture);

    if (const auto it = m_impl->sprites.find(name); it != m_impl->sprites.end())
    {
        it->second.setTexture(m_impl->textures[name]);
    }
    else
    {
        throw std::runtime_error("Sprite not found: " + name);
    }
}

void eng::SFMLRenderer::setSpriteFrame(const std::string &name, int fx, int fy, int fnx, int fny)
{
    if (const auto it = m_impl->sprites.find(name); it != m_impl->sprites.end())
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
    if (const auto it = m_impl->sprites.find(name); it != m_impl->sprites.end())
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
    m_impl->window.draw(&point, 1, sf::PrimitiveType::Points);
}

eng::WindowSize eng::SFMLRenderer::getWindowSize()
{
    const sf::Vector2u size = m_impl->window.getSize();
    return {.width = size.x, .height = size.y};
}