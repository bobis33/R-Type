///
/// @file ServerScene.hpp
/// @brief This file contains the server configuration scene
/// @namespace cli
///

#pragma once

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace cli
{
    ///
    /// @class ServerScene
    /// @brief Server configuration scene
    /// @namespace cli
    ///
    class ServerScene final : public eng::AScene
    {
        public:
            ServerScene(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                        const std::shared_ptr<eng::IAudio> &audio);
            ~ServerScene() override = default;

            ServerScene(const ServerScene &other) = delete;
            ServerScene &operator=(const ServerScene &other) = delete;
            ServerScene(ServerScene &&other) = delete;
            ServerScene &operator=(ServerScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void(const std::string &playerName, const std::string &serverIP,
                               const std::string &serverPort)>
                onConnect;
            std::function<void()> onBackToMenu;

        private:
            const std::shared_ptr<eng::IAudio> &m_audio;

            size_t m_selectedIndex = 0;
            float m_animationTime = 0.0f;
            bool m_isEditing = false;
            const std::vector<std::string> m_serverOptions = {"Player Name", "Server IP", "Server Port", "Connect",
                                                              "Back"};

            void connectServer(const std::string &playerName, const std::string &serverIP,
                               const std::string &serverPort);

            std::string m_playerName = "Player";
            std::string m_serverIP = "0.0.0.0";
            std::string m_serverPort = "2560";

            ecs::Entity m_titleEntity = 0;
            ecs::Entity m_playerNameValueEntity = 0;
            ecs::Entity m_serverIPValueEntity = 0;
            ecs::Entity m_serverPortValueEntity = 0;

            std::string &getCurrentEditField();
            void updateValueDisplay();
    }; // class ServerScene
} // namespace cli
