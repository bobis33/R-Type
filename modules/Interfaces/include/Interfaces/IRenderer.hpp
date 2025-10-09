///
/// @file IRenderer.hpp
/// @brief This file contains the IRenderer class declaration
/// @namespace eng
///

#pragma once

#include <string>

#include "Utils/Interfaces/IPlugin.hpp"

namespace eng
{
    struct Color
    {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
    };
    struct Text
    {
            std::string font_name;
            Color color;
            std::string content;
            unsigned int size;
            float x;
            float y;
            std::string name;
    };

    enum class Key
    {
        Unknown,
        Escape,
        Enter,
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
            unsigned int width;
            unsigned int height;
    };

    ///
    /// @class IRenderer
    /// @brief Interface for the renderer
    /// @namespace eng
    ///
    class IRenderer : public utl::IPlugin
    {

        public:
            virtual void createWindow(const std::string &title, unsigned int height, unsigned int width,
                                      unsigned int frameLimit, bool fullscreen) = 0;
            [[nodiscard]] virtual bool windowIsOpen() const = 0;
            virtual void closeWindow() = 0;
            virtual void clearWindow(Color color) = 0;
            virtual void displayWindow() = 0;
            [[nodiscard]] virtual WindowSize getWindowSize() = 0;

            [[nodiscard]] virtual bool pollEvent(Event &event) = 0;
            virtual void setFrameLimit(unsigned int frameLimit) = 0;

            virtual void createFont(const std::string &name, const std::string &path) = 0;
            virtual void createText(Text text) = 0;
            virtual void drawText(const std::string &name) = 0;
            virtual void setTextContent(const std::string &name, const std::string &content) = 0;
            virtual void setTextPosition(const std::string &name, float x, float y) = 0;
            virtual void setTextColor(const std::string &name, Color color) = 0;

            virtual void createTexture(const std::string &name, const std::string &path) = 0;
            virtual void createSprite(const std::string &name, const std::string &textureName, float x, float y,
                                      float scale_x = 1, float scale_y = 1, int fx = 0, int fy = 0, int fnx = -1,
                                      int fny = -1) = 0;
            virtual void drawSprite(const std::string &name) = 0;
            virtual void setSpritePosition(const std::string &name, float x, float y) = 0;
            virtual void setSpriteTexture(const std::string &name, const std::string &path) = 0;
            virtual void setSpriteScale(const std::string &name, int x, int y) = 0;
            virtual void setSpriteFrame(const std::string &name, int fx, int fy, int fnx, int fny) = 0;

            virtual void drawPoint(float x, float y, Color color) = 0;

        private:
    }; // class IRenderer

} // namespace eng