///
/// @file RTypeClientSolo.hpp
/// @brief This file contains the RTypeClientSolo class declaration
/// @namespace gme
///

#pragma once

#include "Interfaces/IGameClient.hpp"

namespace gme
{

    ///
    /// @class RTypeClientSolo
    /// @brief Class for the RTypeClientSolo plugin
    /// @namespace gme
    ///
    class RTypeClientSolo final : public IGameClient
    {

        public:
            RTypeClientSolo() = default;
            ~RTypeClientSolo() override = default;

            RTypeClientSolo(const RTypeClientSolo &) = delete;
            RTypeClientSolo &operator=(const RTypeClientSolo &) = delete;
            RTypeClientSolo(RTypeClientSolo &&) = delete;
            RTypeClientSolo &operator=(RTypeClientSolo &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "RType_Client_Solo"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::GAME_CLIENT; }

            void update(float deltaTime, unsigned int width, unsigned int height) override;
            void init(eng::Engine &engine, float &audioVolume, const int skinIndex, bool &showDebug,
                      const eng::id menuSceneId) override
            {
                m_engine = &engine;
                m_audioVolume = audioVolume;
                m_skinIndex = skinIndex;
                setupScenes(showDebug, menuSceneId);
            }

            [[nodiscard]] unsigned int getMainSceneId() const override { return m_mainSceneId; }

        private:
            eng::Engine *m_engine = nullptr;
            float m_audioVolume;
            int m_skinIndex;
            eng::id m_mainSceneId;

            void setupScenes(bool &showDebug, eng::id menuSceneId);
    }; // class RTypeClientSolo

} // namespace gme