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
            ServerScene(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio);
            ~ServerScene() override = default;

            ServerScene(const ServerScene &other) = delete;
            ServerScene &operator=(const ServerScene &other) = delete;
            ServerScene(ServerScene &&other) = delete;
            ServerScene &operator=(ServerScene &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;

            std::function<void(const std::string &playerName, const std::string &serverIP, const std::string &serverPort)> onConnect;
            std::function<void()> onBackToMenu;

        private:
            const std::shared_ptr<eng::IAudio> &m_audio;

            size_t m_selectedIndex = 0;
            const std::vector<std::string> m_serverOptions = {"Player Name", "Server IP", "Server Port", "Connect", "Back to Menu"};

            std::string m_playerName = "";
            std::string m_serverIP = "";
            std::string m_serverPort = "";

            ecs::Entity m_playerNameValueEntity;
            ecs::Entity m_serverIPValueEntity;
            ecs::Entity m_serverPortValueEntity;
            ecs::Entity m_titleEntity;
    }; // class ServerScene
} // namespace cli