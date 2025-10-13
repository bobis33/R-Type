///
/// @file RTypeGameLocal.cpp
/// @brief R-Type Local/Solo Game Plugin implementation
///

#include "RTypeGameLocal/RTypeGameLocal.hpp"
#include "RTypeGame/Scenes/Menu.hpp"
#include "RTypeGame/Scenes/GameSolo.hpp"
#include "Utils/Logger.hpp"

gme::RTypeGameLocal::RTypeGameLocal()
{
    m_name = "R-Type Local";
    utl::Logger::log("RTypeGameLocal plugin initialized", utl::LogLevel::INFO);
}

void gme::RTypeGameLocal::initializeScenes()
{
    // Create and add scenes to the scene manager
    auto menu = std::make_unique<Menu>(m_renderer, m_audio);
    auto gameSolo = std::make_unique<GameSolo>(m_renderer, m_audio);
    
    const auto menuId = menu->getId();
    const auto gameSoloId = gameSolo->getId();
    
    // Setup menu callbacks for local game
    menu->onOptionSelected = [this, gameSoloId](const std::string &option)
    {
        utl::Logger::log("Local game - Menu option selected: " + option, utl::LogLevel::INFO);
        
        if (option == "Solo")
        {
            utl::Logger::log("Switching to solo game scene...", utl::LogLevel::INFO);
            m_sceneManager->switchToScene(gameSoloId);
        }
        else if (option == "Settings")
        {
            // TODO: Switch to settings scene
            utl::Logger::log("Opening settings...", utl::LogLevel::INFO);
        }
    };
    
    m_sceneManager->addScene(std::move(menu));
    m_sceneManager->addScene(std::move(gameSolo));
    m_sceneManager->switchToScene(menuId);
}

