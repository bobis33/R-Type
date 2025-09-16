///
/// @file SFMLRenderer.hpp
/// @brief SFMLRenderer class declaration with PImpl
/// @namespace eng
///

#pragma once

#include <memory>
#include <string>

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
            void setFrameLimit(unsigned int frameLimit) override;
            bool windowIsOpen() const override;
            void closeWindow() override;

            void createText(Text text) override;
            void createFont(Font font) override;
            void setTextContent(const std::string &name, const std::string &content) override;
            void setTextPosition(const std::string &name, int x, int y) override;
            void setTextColor(const std::string &name, Color color) override;
            void drawText(const std::string &name) override;

            void clearWindow(Color color) override;
            void displayWindow() override;

            bool pollEvent(Event &event) override;

        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    }; // class SFMLRenderer

} // namespace eng
