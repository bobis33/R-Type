///
/// @file RTypeClientMulti.hpp
/// @brief This file contains the RTypeClientMulti class declaration
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameClient.hpp"

namespace gme
{

    ///
    /// @class RTypeClientMulti
    /// @brief Class for the RTypeClientMulti plugin
    /// @namespace gme
    ///
    class RTypeClientMulti final : public IGameClient
    {

        public:
            RTypeClientMulti() = default;
            ~RTypeClientMulti() override = default;

            RTypeClientMulti(const RTypeClientMulti &) = delete;
            RTypeClientMulti &operator=(const RTypeClientMulti &) = delete;
            RTypeClientMulti(RTypeClientMulti &&) = delete;
            RTypeClientMulti &operator=(RTypeClientMulti &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "RType_Client_Multi"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::GAME_CLIENT; }

            void init(eng::Engine &engine, float &audioVolume, const int skinIndex, bool &showDebug,
                      const eng::id menuSceneId) override
            {
                m_engine = &engine;
                m_audioVolume = audioVolume;
                m_skinIndex = skinIndex;
                m_showDebug = showDebug;
                setupScenes(showDebug, menuSceneId);
            }
            void update(float deltaTime, unsigned int width, unsigned int height) override;
            [[nodiscard]] unsigned int getMainSceneId() const override { return m_mainSceneId; }

        private:
            void setupScenes(bool &showDebug, eng::id menuSceneId);

            eng::Engine *m_engine = nullptr;
            float m_audioVolume{};
            int m_skinIndex{};
            bool m_showDebug = false;
            eng::id m_mainSceneId = 1;

    }; // class RTypeClientMulti

} // namespace gme