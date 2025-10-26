///
/// @file ISystems.hpp
/// @brief This file contains the interface for systems
/// @namespace ecs
///

#pragma once

#include "ECS/Registry.hpp"

namespace ecs
{

    ///
    /// @interface ISystem
    /// @brief Interface class for system
    /// @namespace ecs
    ///
    class ISystem
    {
        public:
            virtual ~ISystem() = default;
            virtual void update(Registry &registry, float dt) = 0;
            virtual bool isEnable() = 0;
            virtual void setEnable(bool enable) = 0;
    };

    ///
    /// @class ASystem
    /// @brief Abstract class for system
    /// @namespace ecs
    ///
    class ASystem : public ISystem
    {
        public:
            bool isEnable() override { return m_isEnable; }
            void setEnable(const bool enable) override { m_isEnable = enable; }

        private:
            bool m_isEnable = true;
    };

} // namespace ecs