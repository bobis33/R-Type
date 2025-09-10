///
/// @file Renderer.hpp
/// @brief This file contains the Renderer class declaration
/// @namespace rtp
///

#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "R-Type/Renderer/EventHandler.hpp"
#include "Utils/Clock.hpp"

// Forward declaration
namespace sf
{
    class RenderWindow;
}

namespace rtp
{

    ///
    /// @class Renderer
    /// @brief Class for the renderer
    /// @namespace rtp
    ///
    class Renderer
    {

        public:
            Renderer(unsigned int height, unsigned int width, unsigned int frameLimit, bool fullscreen);
            ~Renderer() = default;

            Renderer(const Renderer &) = delete;
            Renderer &operator=(const Renderer &) = delete;
            Renderer(Renderer &&) = delete;
            Renderer &operator=(Renderer &&) = delete;

            void run();

            EventHandler &getEventHandler() { return m_eventHandler; }

        private:
            sf::RenderWindow m_window;
            sf::Font m_mainFont;
            sf::Music m_backgroundMusic;
            EventHandler m_eventHandler;
            utl::Clock m_clock;
    }; // class Renderer

} // namespace rtp