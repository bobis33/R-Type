///
/// @file IScene.hpp
/// @brief This file contains the IScene class
/// @namespace eng
///

#pragma once

#include <string>

#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace eng
{

    using id = unsigned int;

    ///
    /// @class IScene
    /// @brief interface class for scene
    /// @namespace eng
    ///
    class IScene
    {
        public:
            virtual ~IScene() = default;

            [[nodiscard]] virtual std::string &getName() = 0;
            [[nodiscard]] virtual id getId() const = 0;
            [[nodiscard]] virtual ecs::Registry &getRegistry() = 0;

            virtual void setName(const std::string &newName) = 0;

            virtual void setEventHandler(std::function<void(const Event&)> handler) = 0;
            virtual void onEvent(const Event &event) = 0;

            virtual void setUpdateHandler(std::function<void(float)> handler) = 0;
            virtual void onUpdate(float dt) = 0;
    }; // class IScene

    ///
    /// @class Scene
    /// @brief Class for scene
    /// @namespace eng
    ///
    class Scene final : public IScene
    {
        public:
            Scene() : m_id(s_nextId++) {}
            ~Scene() override = default;

            [[nodiscard]] std::string &getName() override { return m_name; }
            [[nodiscard]] id getId() const override { return m_id; }
            [[nodiscard]] ecs::Registry &getRegistry() override { return m_registry; }

            void setName(const std::string &newName) override { m_name = newName; }

            void setEventHandler(std::function<void(const Event&)> handler) override { m_eventHandler = std::move(handler); }
            void onEvent(const Event &event) override {
                if (m_eventHandler) {
                    m_eventHandler(event);
                }
            }
            void setUpdateHandler(std::function<void(float)> handler) override { m_updateHandler = std::move(handler); }
            void onUpdate(const float dt) override
            {
                if (m_updateHandler)
                {
                    m_updateHandler(dt);
                }
            }

        private:
            std::string m_name = "default_name";
            id m_id;
            ecs::Registry m_registry;
            inline static id s_nextId = 1;
            std::function<void(const Event&)> m_eventHandler;
            std::function<void(float)> m_updateHandler;

    }; // class Scene

} // namespace eng