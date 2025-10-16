#include "Client/Client.hpp"
#include "ECS/Component.hpp"

void cli::Client::handleEvents(eng::Event &event)
{
    const auto &scene = m_engine->getSceneManager()->getCurrentScene();

    while (m_engine->getRenderer()->pollEvent(event))
    {
        scene->event(event);

        switch (event.type)
        {
            case eng::EventType::Closed:
                m_engine->setState(eng::State::STOP);
                break;

            case eng::EventType::KeyPressed:
                // if (event.key == eng::Key::Escape)
                //{
                //     m_engine->getRenderer()->closeWindow();
                //     m_engine->setState(eng::State::STOP);
                // }
                // else
                //{
                m_keysPressed[event.key] = true;
                //}
                break;

            case eng::EventType::KeyReleased:
                m_keysPressed[event.key] = false;
                break;

            default:
                break;
        }
    }

    updateKeyboardInput(scene->getRegistry());
}

void cli::Client::updateKeyboardInput(ecs::Registry &registry)
{
    auto keyboardEntities = registry.getAll<ecs::KeyboardInput>();
    ecs::Entity keyboardEntity;
    
    if (keyboardEntities.empty()) {
        keyboardEntity = registry.createEntity()
            .with<ecs::KeyboardInput>("keyboard_input")
            .build();
    }
    else
    {
        keyboardEntity = keyboardEntities.begin()->first;
    }
    auto *keyboardInput = registry.getComponent<ecs::KeyboardInput>(keyboardEntity);
    if (keyboardInput) {
        keyboardInput->space_pressed = m_keysPressed[eng::Key::Space];
        keyboardInput->up_pressed = m_keysPressed[eng::Key::Up];
        keyboardInput->down_pressed = m_keysPressed[eng::Key::Down];
        keyboardInput->left_pressed = m_keysPressed[eng::Key::Left];
        keyboardInput->right_pressed = m_keysPressed[eng::Key::Right];
    }
}