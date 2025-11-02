///
/// @file RTypeClientMulti.hpp
/// @brief R-Type multiplayer game client plugin implementation
/// @details This file contains the main game client class for R-Type multiplayer mode.
///          It manages game scenes, network integration, and multiplayer gameplay logic.
///          The plugin implements lobby system, player synchronization, and multiplayer-specific
///          game mechanics for the R-Type game.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "Interfaces/IGameClient.hpp"

namespace gme
{

    ///
    /// @class RTypeClientMulti
    /// @brief Main multiplayer game client plugin for R-Type
    /// @details This class serves as the entry point for the R-Type multiplayer client plugin.
    ///          It manages:
    ///          - Game scene initialization and transitions
    ///          - Network client integration
    ///          - Lobby system UI and logic
    ///          - Multiplayer game state
    ///          - Player synchronization
    ///
    /// The plugin integrates with the engine's plugin system and provides a complete
    /// multiplayer game experience including server browser, lobby system, and networked gameplay.
    ///
    /// @namespace gme
    ///
    class RTypeClientMulti final : public IGameClient
    {

        public:
            ///
            /// @brief Default constructor
            ///
            RTypeClientMulti() = default;

            ///
            /// @brief Destructor
            ///
            ~RTypeClientMulti() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            RTypeClientMulti(const RTypeClientMulti &) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            RTypeClientMulti &operator=(const RTypeClientMulti &) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            RTypeClientMulti(RTypeClientMulti &&) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            RTypeClientMulti &operator=(RTypeClientMulti &&) = delete;

            ///
            /// @brief Get plugin name
            /// @return Plugin name identifier
            ///
            [[nodiscard]] const std::string getName() const override { return "RType_Client_Multi"; }

            ///
            /// @brief Get plugin type
            /// @return Plugin type (GAME_CLIENT)
            ///
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::GAME_CLIENT; }

            ///
            /// @brief Initialize the multiplayer client plugin
            /// @param engine Reference to the game engine
            /// @param appConfig Application configuration
            /// @param showDebug Debug mode flag
            /// @param menuSceneId ID of the main menu scene to return to
            /// @details Sets up all multiplayer-specific scenes including:
            ///          - Server connection scene
            ///          - Lobby browser
            ///          - Lobby creation
            ///          - Waiting room
            ///          - Multiplayer game scene
            ///          - Game over scene
            ///
            void init(eng::Engine &engine, utl::cli::AppConfig &appConfig, bool &showDebug,
                      const eng::id menuSceneId) override
            {
                m_engine = &engine;
                m_appConfig = &appConfig;
                m_showDebug = showDebug;
                setupScenes(showDebug, menuSceneId);
            }
            ///
            /// @brief Update the multiplayer client plugin (called each frame)
            /// @param deltaTime Time elapsed since last frame (in seconds)
            /// @param width Current window width
            /// @param height Current window height
            /// @details Performs frame-by-frame updates for multiplayer functionality
            ///
            void update(float deltaTime, unsigned int width, unsigned int height) override;

            ///
            /// @brief Get the main scene ID
            /// @return ID of the main multiplayer scene
            ///
            [[nodiscard]] unsigned int getMainSceneId() const override { return m_mainSceneId; }

        private:
            ///
            /// @brief Setup all multiplayer game scenes
            /// @param showDebug Debug mode flag
            /// @param menuSceneId ID of the main menu scene
            /// @details Creates and registers all scenes required for multiplayer gameplay:
            ///          - Server connection and configuration
            ///          - Lobby system (browse, create, join)
            ///          - Waiting room
            ///          - Game scene
            ///          - Game over screen
            ///
            void setupScenes(bool &showDebug, eng::id menuSceneId);

            eng::Engine *m_engine = nullptr;            ///< Pointer to the game engine
            utl::cli::AppConfig *m_appConfig = nullptr; ///< Pointer to application configuration
            bool m_showDebug = false;                   ///< Debug mode flag
            eng::id m_mainSceneId = 1;                  ///< ID of the main multiplayer scene

    }; // class RTypeClientMulti

} // namespace gme
