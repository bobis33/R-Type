///
/// @file IRenderer.hpp
/// @brief This file contains the IRenderer class declaration
/// @namespace eng
///

#pragma once

#include <string>

namespace eng
{
    struct Audio
    {
            std::string path;
            float volume;
            bool loop;
            std::string name;
    };
    struct Color
    {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
    };
    struct Font
    {
            std::string path;
            std::string name;
    };
    struct Text
    {
            std::string fontName;
            Color color;
            std::string content;
            int size;
            int x;
            int y;
            std::string name;
    };

    enum class Key
    {
        Unknown,
        Escape,
        Space,
        Up,
        Down,
        Left,
        Right,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9
    };
    enum class EventType
    {
        Closed,
        KeyPressed,
        KeyReleased,
        None
    };

    struct Event
    {
            EventType type = EventType::None;
            Key key = Key::Unknown;
    };

    struct WindowSize
    {
            int width;
            int height;
    };

    ///
    /// @class IRenderer
    /// @brief Interface for the renderer
    /// @namespace cli
    ///
    class IRenderer
    {

        public:
            virtual ~IRenderer() = default;

            virtual void createWindow(const std::string &title, unsigned int height, unsigned int width,
                                      unsigned int frameLimit, bool fullscreen) = 0;
            [[nodiscard]] virtual bool windowIsOpen() const = 0;
            virtual void closeWindow() = 0;

            virtual void createFont(Font font) = 0;
            virtual void createText(Text text) = 0;
            virtual void createSprite(const std::string &path, int x, int y, const std::string &name, float scale_x = 1,
                                      float scale_y = 1, int fx = 0, int fy = 0, int fnx = -1, int fny = -1) = 0;
            virtual void drawText(const std::string &name) = 0;
            virtual void drawSprite(const std::string &name) = 0;
            virtual void setTextContent(const std::string &name, const std::string &content) = 0;
            virtual void setTextPosition(const std::string &name, int x, int y) = 0;
            virtual void setTextColor(const std::string &name, Color color) = 0;
            virtual void setSpritePosition(const std::string &name, int x, int y) = 0;
            virtual void setSpriteTexture(const std::string &name, const std::string &path) = 0;
            virtual void setSpriteScale(const std::string &name, int x, int y) = 0;
            virtual void setSpriteFrame(const std::string &name, int fx, int fy, int fnx, int fny) = 0;
            virtual void setFrameLimit(unsigned int frameLimit) = 0;

            virtual void clearWindow(Color color) = 0;
            virtual void displayWindow() = 0;

            virtual void drawPoint(int x, int y, Color color) = 0;
            virtual void drawLine(int x1, int x2, int y1, int y2, Color color) = 0;

            virtual WindowSize getWindowSize() = 0;

            [[nodiscard]] virtual bool pollEvent(Event &event) = 0;

        private:
    }; // class IRenderer

} // namespace eng