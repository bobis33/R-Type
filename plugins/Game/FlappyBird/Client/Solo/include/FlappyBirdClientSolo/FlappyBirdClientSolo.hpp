///
/// @file FlappyBirdClientSolo.hpp
/// @brief This file contains the FlappyBirdClientSolo class declaration
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameClient.hpp"

namespace gme
{

    ///
    /// @class FlappyBirdClientSolo
    /// @brief Class for the FlappyBirdClientSolo plugin
    /// @namespace gme
    ///
    class FlappyBirdClientSolo final : public IGameClient
    {

        public:
            FlappyBirdClientSolo() = default;
            ~FlappyBirdClientSolo() override = default;

            FlappyBirdClientSolo(const FlappyBirdClientSolo &) = delete;
            FlappyBirdClientSolo &operator=(const FlappyBirdClientSolo &) = delete;
            FlappyBirdClientSolo(FlappyBirdClientSolo &&) = delete;
            FlappyBirdClientSolo &operator=(FlappyBirdClientSolo &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "RType_Client_Solo"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::GAME_CLIENT; }

            void update(float deltaTime, unsigned int width, unsigned int height) override;
            void init(eng::Engine &engine, utl::cli::AppConfig &appConfig, bool &showDebug,
                      const eng::id menuSceneId, const eng::id winSceneId) override
            {
                m_engine = &engine;
                m_appConfig = &appConfig;
                m_showDebug = &showDebug;
                setupScenes(showDebug, menuSceneId);
            }

            [[nodiscard]] unsigned int getMainSceneId() const override { return m_mainSceneId; }

        private:
            eng::Engine *m_engine = nullptr;
            utl::cli::AppConfig *m_appConfig = nullptr;
            eng::id m_mainSceneId;
            bool *m_showDebug = nullptr;

            void setupScenes(bool &showDebug, eng::id menuSceneId);
    }; // class FlappyBirdClientSolo
} // namespace gme