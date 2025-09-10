#include "R-Type/Renderer/Renderer.hpp"
#include "R-Type/Common.hpp"
#include "Utils/Logger.hpp"

static constexpr std::string_view WINDOW_TITLE = "R-Type - Client";

rtp::Renderer::Renderer(unsigned int height, unsigned int width, const unsigned int frameLimit, const bool fullscreen)
{
    sf::VideoMode mode;

    if (fullscreen)
    {
        mode = sf::VideoMode::getDesktopMode();
    }
    else
    {
        mode = sf::VideoMode({width, height});
    }

    m_window.create(mode, WINDOW_TITLE, fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
    m_window.setFramerateLimit(frameLimit);
    if (!m_mainFont.openFromFile(Paths::Fonts::FONTS_RTYPE))
    {
        utl::Logger::log("Failed to load font", utl::LogLevel::WARNING);
    }
    if (!m_backgroundMusic.openFromFile(Paths::Audio::AUDIO_TITLE))
    {
        utl::Logger::log("Failed to load audio file", utl::LogLevel::WARNING);
    }
    m_backgroundMusic.setLooping(true);
    m_backgroundMusic.play();
    m_eventHandler.subscribe<sf::Event::Closed>([&](const sf::Event::Closed &) { m_window.close(); });
    m_eventHandler.subscribe<sf::Event::KeyPressed>(
        [&](const sf::Event::KeyPressed &key)
        {
            utl::Logger::log(std::string("Key pressed: ") + std::to_string(static_cast<int>(key.scancode)),
                             utl::LogLevel::INFO);
            if (key.scancode == sf::Keyboard::Scancode::Escape)
            {
                m_window.close();
            }
        });
}

void rtp::Renderer::run()
{
    sf::Text title(m_mainFont);
    sf::Text fpsText(m_mainFont);
    title.setString("RType Client");
    fpsText.setString("RType Client");
    title.setCharacterSize(50);
    fpsText.setCharacterSize(20);
    title.setFillColor(sf::Color::White);
    fpsText.setFillColor(sf::Color::White);
    title.setPosition({10.0F, 10.0F});
    fpsText.setPosition({10.0F, 70.0F});
    while (m_window.isOpen())
    {
        fpsText.setString("FPS " + std::to_string(static_cast<int>(1.0F / m_clock.getDeltaSeconds())));
        m_clock.restart();
        while (auto eventOpt = m_window.pollEvent())
        {
            m_eventHandler.publish(*eventOpt);
        }
        m_window.clear(sf::Color::Black);
        m_window.draw(title);
        m_window.draw(fpsText);
        m_window.display();
    }
}
