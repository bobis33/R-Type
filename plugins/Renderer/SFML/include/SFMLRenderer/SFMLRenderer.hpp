///
/// @file SFMLRenderer.hpp
/// @brief This file contains the SFMLRenderer class declaration
/// @namespace eng
///

#pragma once

#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "Interfaces/IRenderer.hpp"

namespace eng
{

    ///
    /// @class SFMLRenderer
    /// @brief Class for the SFMLRenderer plugin
    /// @namespace eng
    ///
    class SFMLRenderer final : public IRenderer
    {

        public:
            SFMLRenderer() = default;
            ~SFMLRenderer() override = default;

            SFMLRenderer(const SFMLRenderer &) = delete;
            SFMLRenderer &operator=(const SFMLRenderer &) = delete;
            SFMLRenderer(SFMLRenderer &&) = delete;
            SFMLRenderer &operator=(SFMLRenderer &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Renderer_SFML"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::RENDERER; }

            void createWindow(const std::string &title, unsigned int height, unsigned int width,
                              unsigned int frameLimit, bool fullscreen) override;
            bool windowIsOpen() const override;
            void closeWindow() override;
            void clearWindow(Color color) override;
            void displayWindow() override;
            WindowSize getWindowSize() override;

            bool pollEvent(Event &event) override;
            void setFrameLimit(unsigned int frameLimit) override;

            void createFont(const std::string &name, const std::string &path) override;
            void createText(Text text) override;
            void setTextContent(const std::string &name, const std::string &content) override;
            void setTextPosition(const std::string &name, float x, float y) override;
            void setTextColor(const std::string &name, Color color) override;

            void drawText(const std::string &name) override;
            void createTexture(const std::string &name, const std::string &path) override;
            void createSprite(const std::string &name, const std::string &textureName, float x, float y, float scale_x,
                              float scale_y, int fx, int fy, int fnx, int fny) override;
            void setSpritePosition(const std::string &name, float x, float y) override;
            void setSpriteTexture(const std::string &name, const std::string &path) override;
            void setSpriteFrame(const std::string &name, int fx, int fy, int fnx, int fny) override;
            void setSpriteScale(const std::string &name, int x, int y) override;
            void drawSprite(const std::string &name) override;

            void drawPoint(float x, float y, Color color) override;

        private:
            std::unordered_map<std::string, sf::Texture> textures;

            sf::RenderWindow window;
            std::unordered_map<std::string, sf::Font> fonts;
            std::unordered_map<std::string, sf::Text> texts;
            std::unordered_map<std::string, sf::Sprite> sprites;

    }; // class SFMLRenderer

} // namespace eng