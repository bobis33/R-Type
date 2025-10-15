///
/// @file RTypeGameMulti.cpp
/// @brief R-Type Multiplayer Game Plugin implementation
///

#include "RTypeGameMulti/RTypeGameMulti.hpp"
#include "RTypeGame/Scenes/Menu.hpp"
#include "Utils/Logger.hpp"

gme::RTypeGameMulti::RTypeGameMulti()
{
    m_name = "R-Type Multiplayer";
    utl::Logger::log("RTypeGameMulti plugin initialized", utl::LogLevel::INFO);
}

void gme::RTypeGameMulti::initializeScenes()
{
    // Create and add scenes to the scene manager
    auto menu = std::make_unique<Menu>(m_renderer, m_audio);
    const auto menuId = menu->getId();
    
    // Setup menu callbacks for multiplayer game
    menu->onOptionSelected = [this, menuId](const std::string &option)
    {
        utl::Logger::log("Multiplayer game - Menu option selected: " + option, utl::LogLevel::INFO);
        
        if (option == "Multi")
        {
            // TODO: Switch to multiplayer lobby/room selection
            utl::Logger::log("Opening multiplayer lobby...", utl::LogLevel::INFO);
        }
        else if (option == "Settings")
        {
            // TODO: Switch to settings scene
            utl::Logger::log("Opening settings...", utl::LogLevel::INFO);
        }
    };
    
    m_sceneManager->addScene(std::move(menu));
    m_sceneManager->switchToScene(menuId);
}

