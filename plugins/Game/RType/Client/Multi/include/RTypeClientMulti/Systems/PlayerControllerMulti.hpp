///
/// @file PlayerControllerMulti.hpp
/// @brief Player Controller System for multiplayer (sends inputs to server)
/// @namespace gme
///

#pragma once

#include <deque>

#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Utils/Common.hpp"
#include "Utils/EventBus.hpp"

namespace gme
{
    class PlayerControllerMulti final : public ecs::ASystem
    {
        public:
            explicit PlayerControllerMulti(const std::shared_ptr<eng::IRenderer> &renderer, uint32_t sessionId)
                : m_renderer(renderer), m_sessionId(sessionId), m_eventBus(utl::EventBus::getInstance())
            {
                m_componentId = 11;
                m_eventBus.registerComponent(m_componentId, "PlayerControllerMulti");
            }
            ~PlayerControllerMulti() override = default;

            PlayerControllerMulti(const PlayerControllerMulti &) = delete;
            PlayerControllerMulti &operator=(const PlayerControllerMulti &) = delete;
            PlayerControllerMulti(PlayerControllerMulti &&) = delete;
            PlayerControllerMulti &operator=(PlayerControllerMulti &&) = delete;

            void update(ecs::Registry &registry, float dt) override;
            void handleInput(ecs::Registry &registry, const eng::Event &event);
            ecs::Entity createPlayer(ecs::Registry &registry, float x, float y);

            bool isSpacePressed() const;

        private:
            void sendInputToServer(bool up, bool down, bool left, bool right, bool shoot, bool die);
            void sendInputsIfChanged();

            const std::shared_ptr<eng::IRenderer> &m_renderer;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_playerEntity;
            uint32_t m_sessionId;
            uint32_t m_componentId;
            utl::EventBus &m_eventBus;

            float m_lastInputSendTime = 0.0f;
            const float INPUT_THROTTLE_INTERVAL = 1.0f / 144.0f; // Send inputs at 144 Hz max

            // Input sequence tracking for prediction + reconciliation
            uint32_t m_nextSeqId = 1;
            uint32_t m_lastAckSeqId = 0;

            struct PendingInput
            {
                    uint32_t seqId;
                    std::vector<uint8_t> inputData;
                    float dt;
            };
            std::deque<PendingInput> m_inputHistory;
    }; // class PlayerControllerMulti
} // namespace gme
