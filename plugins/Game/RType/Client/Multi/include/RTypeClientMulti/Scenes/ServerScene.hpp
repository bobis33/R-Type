///
/// @file ServerScene.hpp
/// @brief This file contains the server configuration scene
/// @namespace gme
///

#pragma once

#include "Engine/Interfaces/IScene.hpp"

namespace gme
{
    ///
    /// @class ServerScene
    /// @brief Server configuration scene
    /// @namespace gme
    ///
    class ServerScene final : public eng::AScene
    {
        public:
            ServerScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer);
            ~ServerScene() override = default;

            ServerScene(const ServerScene &other) = delete;
            ServerScene &operator=(const ServerScene &other) = delete;
            ServerScene(ServerScene &&other) = delete;
            ServerScene &operator=(ServerScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            bool &playMusic() { return m_playMusic; }

            std::function<void(const std::string &playerName, const std::string &serverIP,
                               const std::string &serverPort)>
                onConnect;
            std::function<void()> onBackToMenu;

        private:
            size_t m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            bool m_isEditing = false;
            const std::vector<std::string> m_serverOptions = {"Player Name", "Server IP", "Server Port", "Connect",
                                                              "Back"};

            void connectServer(const std::string &playerName, const std::string &serverIP,
                               const std::string &serverPort) const;

            std::string m_playerName = "Player";
            std::string m_serverIP = "127.0.0.1";
            std::string m_serverPort = "2560";

            ecs::Entity m_titleEntity = 0;
            ecs::Entity m_playerNameValueEntity = 0;
            ecs::Entity m_serverIPValueEntity = 0;
            ecs::Entity m_serverPortValueEntity = 0;
            bool m_playMusic = false;
            std::string &getCurrentEditField();
            void updateValueDisplay();
    }; // class ServerScene
} // namespace gme
