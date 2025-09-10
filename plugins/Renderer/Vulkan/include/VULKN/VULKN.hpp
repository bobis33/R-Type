///
/// @file VULKN.hpp
/// @brief This file contains the VULKN class declaration
/// @namespace cae
///

#pragma once

namespace cae
{

    ///
    /// @class VULKN
    /// @brief Class for the Vulkan plugin
    /// @namespace cae
    ///
    class VULKN final : public IRenderer
    {

    public:
        VULKN() = default;
        ~VULKN() override = default;

        VULKN(const VULKN &) = delete;
        VULKN &operator=(const VULKN &) = delete;
        VULKN(VULKN &&) = delete;
        VULKN &operator=(VULKN &&) = delete;

        [[nodiscard]] const char* getName() const override { return "Vulkan"; }
        [[nodiscard]] PluginType getType() const override { return PluginType::RENDERER; }

    }; // class VULKN

} // namespace cae