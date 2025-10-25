///
/// @file PlayerController.hpp
/// @brief Player Controller System for handling player movement and input
/// @namespace gme
///

#pragma once

#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Common.hpp"

namespace gme
{

    class PlayerController final : public eng::ASystem
    {
        public:
            explicit PlayerController(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~PlayerController() override = default;

            PlayerController(const PlayerController &) = delete;
            PlayerController &operator=(const PlayerController &) = delete;
            PlayerController(PlayerController &&) = delete;
            PlayerController &operator=(PlayerController &&) = delete;

            void update(ecs::Registry &registry, float dt) override;
            void handleInput(ecs::Registry &registry, const eng::Event &event);
            ecs::Entity createPlayer(ecs::Registry &registry, float x, float y);

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_playerEntity;
    }; // class PlayerController

} // namespace gme
