///
/// @file AGamePlugin.cpp
/// @brief Base class implementation for R-Type game plugins
///

#include "RTypeGame/AGamePlugin.hpp"
#include "Utils/Logger.hpp"

void gme::AGamePlugin::initialize(const std::shared_ptr<eng::IRenderer> &renderer, 
                                  const std::shared_ptr<eng::IAudio> &audio)
{
    m_renderer = renderer;
    m_audio = audio;
    m_currentScene = std::make_shared<RTypeGameScene>("Main");
    m_sceneManager = std::make_unique<eng::SceneManager>();
    initializeScenes();
}

void gme::AGamePlugin::update(const float deltaTime, const unsigned int width, const unsigned int height)
{
    // Update current engine scene from scene manager
    if (m_sceneManager && m_sceneManager->getCurrentScene())
    {
        m_sceneManager->getCurrentScene()->update(deltaTime, {width, height});
    }

    // Convert ECS entities to sprites for the interface
    convertEntitiesToSprites();
}

void gme::AGamePlugin::handleEvent(const eng::Event &event)
{
    // Forward events to current scene from scene manager
    if (m_sceneManager && m_sceneManager->getCurrentScene())
    {
        m_sceneManager->getCurrentScene()->event(event);
    }
}

eng::IScene *gme::AGamePlugin::getCurrentEngineScene() const
{
    if (m_sceneManager && m_sceneManager->getCurrentScene())
    {
        return m_sceneManager->getCurrentScene().get();
    }
    return nullptr;
}

void gme::AGamePlugin::convertEntitiesToSprites()
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

