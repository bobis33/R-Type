///
/// @file RTypeGame.cpp
/// @brief R-Type Game Plugin implementation
///

#include "RTypeGame/RTypeGame.hpp"
#include "RTypeGame/Scenes/Menu.hpp"
#include "Utils/Logger.hpp"

gme::RTypeGame::RTypeGame()
{
    m_currentScene = std::make_shared<RTypeGameScene>("Main");
    m_sceneManager = std::make_unique<eng::SceneManager>();
    utl::Logger::log("RTypeGame plugin initialized", utl::LogLevel::INFO);
}

void gme::RTypeGame::initialize(const std::shared_ptr<eng::IRenderer> &renderer, 
                                const std::shared_ptr<eng::IAudio> &audio)
{
    m_renderer = renderer;
    m_audio = audio;
    initializeScenes();
}

void gme::RTypeGame::update(const float deltaTime, const unsigned int width, const unsigned int height)
{
    // Update current engine scene from scene manager
    if (m_sceneManager && m_sceneManager->getCurrentScene())
    {
        m_sceneManager->getCurrentScene()->update(deltaTime, {width, height});
    }

    // Convert ECS entities to sprites for the interface
    convertEntitiesToSprites();
}

void gme::RTypeGame::handleEvent(const eng::Event &event)
{
    // Forward events to current scene from scene manager
    if (m_sceneManager && m_sceneManager->getCurrentScene())
    {
        m_sceneManager->getCurrentScene()->event(event);
    }
}

void gme::RTypeGame::initializeScenes()
{
    // Create and add scenes to the scene manager
    auto menu = std::make_unique<Menu>(m_renderer, m_audio);
    const auto menuId = menu->getId();
    
    // Setup menu callbacks
    menu->onOptionSelected = [this, menuId](const std::string &option)
    {
        utl::Logger::log("Menu option selected: " + option, utl::LogLevel::INFO);
        // TODO: Add scene switching logic when other scenes are added
    };
    
    m_sceneManager->addScene(std::move(menu));
    m_sceneManager->switchToScene(menuId);
}

eng::IScene *gme::RTypeGame::getCurrentEngineScene() const
{
    if (m_sceneManager && m_sceneManager->getCurrentScene())
    {
        return m_sceneManager->getCurrentScene().get();
    }
    return nullptr;
}

void gme::RTypeGame::convertEntitiesToSprites()
{
    // Convert ECS entities to sprites
    // This is a bridge between the ECS system and the IGameClient interface
    m_currentScene->clearEntities();

    if (!m_sceneManager || !m_sceneManager->getCurrentScene())
    {
        return;
    }

    auto &registry = m_sceneManager->getCurrentScene()->getRegistry();
    
    // This would need to iterate through entities and convert them to sprites
    // For now, this is a placeholder implementation
    // You could extract position, texture, etc. from ECS components and create Sprite objects
}

