///
/// @file SFMLRenderer.hpp
/// @brief SFMLRenderer class declaration with PImpl
/// @namespace eng
///

#pragma once

#include <memory>

#include "Interfaces/IRenderer.hpp"

namespace eng
{

    ///
    /// @class SFMLRenderer
    /// @brief Class for the R-Type game
    /// @namespace eng
    ///
    class SFMLRenderer final : public IRenderer
    {
        public:
            SFMLRenderer();
            ~SFMLRenderer() override;

            SFMLRenderer(const SFMLRenderer &) = delete;
            SFMLRenderer &operator=(const SFMLRenderer &) = delete;
            SFMLRenderer(SFMLRenderer &&) = delete;
            SFMLRenderer &operator=(SFMLRenderer &&) = delete;

            void createWindow(const std::string &title, unsigned int height, unsigned int width,
                              unsigned int frameLimit, bool fullscreen) override;
            bool windowIsOpen() const override;
            void closeWindow() override;
            void clearWindow(Color color) override;
            void displayWindow() override;
            WindowSize getWindowSize() override;

            bool pollEvent(Event &event) override;
            void setFrameLimit(unsigned int frameLimit) override;

            void createFont(Font font) override;
            void createText(Text text) override;
            void setTextContent(const std::string &name, const std::string &content) override;
            void setTextPosition(const std::string &name, float x, float y) override;
            void setTextColor(const std::string &name, Color color) override;

            void drawText(const std::string &name) override;
            void createTexture(const std::string &path, const std::string &name) override;
            void createSprite(const std::string &path, float x, float y, const std::string &name, float scale_x,
                              float scale_y, int fx, int fy, int fnx, int fny) override;
            void setSpritePosition(const std::string &name, float x, float y) override;
            void setSpriteTexture(const std::string &name, const std::string &path) override;
            void setSpriteFrame(const std::string &name, int fx, int fy, int fnx, int fny) override;
            void setSpriteScale(const std::string &name, int x, int y) override;
            void drawSprite(const std::string &name) override;

            void drawPoint(float x, float y, Color color) override;

        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    }; // class SFMLRenderer

} // namespace eng
