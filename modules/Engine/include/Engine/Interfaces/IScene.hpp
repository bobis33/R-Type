///
/// @file IScene.hpp
/// @brief This file contains the IScene class
/// @namespace eng
///

#pragma once

#include <string>

#include "ECS/Registry.hpp"
#include "ECS/Interfaces/ISystems.hpp"
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

            virtual void update(float dt, const WindowSize &size) = 0;
            virtual void event(const Event &event) = 0;

            virtual void addSystem(std::unique_ptr<ISystem> system) = 0;
            virtual void updateSystems(float dt) = 0;

    }; // class IScene

    ///
    /// @class AScene
    /// @brief Class for scene
    /// @namespace eng
    ///
    class AScene : public IScene
    {
        public:
            AScene() : m_id(s_nextId++) {}
            ~AScene() override = default;

            AScene(const AScene &other) = delete;
            AScene(AScene &&other) = delete;
            AScene &operator=(const AScene &other) = delete;
            AScene &operator=(AScene &&other) = delete;

            [[nodiscard]] std::string &getName() override { return m_name; }
            [[nodiscard]] id getId() const override { return m_id; }
            [[nodiscard]] ecs::Registry &getRegistry() override { return m_registry; }

            void setName(const std::string &newName) override { m_name = newName; }

            void addSystem(std::unique_ptr<ISystem> system) override { m_systems.emplace_back(std::move(system)); }
            void updateSystems(const float dt) override { for (const auto &system : m_systems) { system->update(m_registry, dt); } }

        private:
            std::string m_name = "default_name";
            id m_id = 1;
            ecs::Registry m_registry;
            inline static id s_nextId = 1;
            std::vector<std::unique_ptr<ISystem>> m_systems;
    }; // class AScene

} // namespace eng