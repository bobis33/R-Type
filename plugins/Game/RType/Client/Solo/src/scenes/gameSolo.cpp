#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientSolo/Scenes/GameSolo.hpp"
#include "RTypeClientSolo/GameConfig.hpp"
#include "RTypeClientSolo/Managers/StageManager.hpp"
#include "RTypeClientSolo/Systems/HUD.hpp"
#include "RTypeClientSolo/Systems/PlayerController.hpp"
#include "RTypeClientSolo/Systems/Starfield.hpp"
#include "Utils/Common.hpp"

gme::GameSolo::GameSolo(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                        const std::shared_ptr<eng::IAudio> &audio, const float skinIndex, bool &showDebug)
    : AScene(assignedId), m_audio(audio), m_renderer(renderer), m_skinIndex(skinIndex), m_showDebug(showDebug)
{
    auto &registry = AScene::getRegistry();

    registry.onComponentAdded(
        [&renderer, &audio, &registry](const ecs::Entity e, const std::type_info &type)
        {
            const auto *audioComp = registry.getComponent<ecs::Audio>(e);
            const auto *colorComp = registry.getComponent<ecs::Color>(e);
            const auto *fontComp = registry.getComponent<ecs::Font>(e);
            const auto *rectComp = registry.getComponent<ecs::Rect>(e);
            const auto *scaleComp = registry.getComponent<ecs::Scale>(e);
            const auto *textComp = registry.getComponent<ecs::Text>(e);
            const auto *textureComp = registry.getComponent<ecs::Texture>(e);
            const auto *transform = registry.getComponent<ecs::Transform>(e);
            const auto *hitBox = registry.getComponent<ecs::Hitbox>(e);

            // if hitBox, createCircleShape from renderer
            if (hitBox && transform)
            {
                float hitboxX = transform->x + hitBox->offsetX;
                float hitboxY = transform->y + hitBox->offsetY;
                renderer->createCircleShape({.name = "hitbox_" + std::to_string(e),
                                             .radius = hitBox->radius,
                                             .color = {.r = 255, .g = 0, .b = 0, .a = 100},
                                             .x = hitboxX,
                                             .y = hitboxY,
                                             .outline_thickness = 1.0f,
                                             .outline_color = {.r = 255, .g = 0, .b = 0, .a = 200}});
            }
            if (type == typeid(ecs::Text))
            {
                if (textComp && transform && fontComp)
                {
                    renderer->createFont(fontComp->id, fontComp->path);
                    renderer->createText(
                        {.font_name = fontComp->id,
                         .color = {.r = colorComp->r, .g = colorComp->g, .b = colorComp->b, .a = colorComp->a},
                         .content = textComp->content,
                         .size = textComp->font_size,
                         .x = transform->x,
                         .y = transform->y,
                         .name = textComp->id});
                }
            }
            else if (type == typeid(ecs::Texture))
            {
                const float scale_x = scaleComp ? scaleComp->x : 1.F;
                const float scale_y = scaleComp ? scaleComp->y : 1.F;

                renderer->createTexture(textureComp->id, textureComp->path);

                if (transform && textureComp)
                {
                    if (rectComp)
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y, scale_x, scale_y, static_cast<int>(rectComp->pos_x),
                                               static_cast<int>(rectComp->pos_y), rectComp->size_x, rectComp->size_y);
                    }
                    else
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y);
                    }
                }
            }
            else if (type == typeid(ecs::Audio))
            {
                if (audioComp)
                {
                    audio->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                       audioComp->id + std::to_string(e));
                }
            }
        });

    registry.createEntity().with<ecs::Score>("score", 0).build();

    m_hudSystem = std::make_unique<HUDSystem>(renderer);
    m_starfieldSystem = std::make_unique<StarfieldSystem>(renderer);
    m_playerController = std::make_unique<PlayerController>(renderer);
    m_stageManager = std::make_unique<StageManager>();

    m_playerEntity = m_playerController->createPlayer(registry, 200.F, 100.F);
    m_hudSystem->createScoreHUD(registry, 10.0f, 10.0f);
    m_starfieldSystem->createStarfield(registry, utl::Config::Window::WINDOW_WIDTH, utl::Config::Window::WINDOW_HEIGHT);
    auto beginSoundEntity = registry.createEntity()
                                .with<ecs::Audio>("game_begin", utl::Path::Audio::AUDIO_BEGIN, 1.0F, false, false)
                                .build();
    if (auto *audioComp = registry.getComponent<ecs::Audio>(beginSoundEntity))
    {
        audioComp->play = true;
    }
}

void gme::GameSolo::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();
    const auto &audios = reg.getAll<ecs::Audio>();

    for (const auto &audio : audios)
    {
        if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
        {
            m_audio->stopAudio(audio.second.id);
        }
    }

    static bool starfieldCreated = false;
    if (!starfieldCreated)
    {
        m_starfieldSystem->createStarfield(reg, static_cast<int>(size.width), static_cast<int>(size.height));
        starfieldCreated = true;
    }
    m_starfieldSystem->update(reg, dt);
    m_hudSystem->update(reg, dt);
    m_playerController->update(reg, dt);
    m_stageManager->update(reg, dt, size);
}

void gme::GameSolo::event(const eng::Event &event)
{
    auto &reg = getRegistry();
    m_playerController->handleInput(reg, event);
}

bool gme::GameSolo::isUpPressed() const { return m_keysPressed.contains(eng::Key::Z) && m_keysPressed.at(eng::Key::Z); }

bool gme::GameSolo::isDownPressed() const
{
    return m_keysPressed.contains(eng::Key::S) && m_keysPressed.at(eng::Key::S);
}

bool gme::GameSolo::isLeftPressed() const
{
    return m_keysPressed.contains(eng::Key::Q) && m_keysPressed.at(eng::Key::Q);
}

bool gme::GameSolo::isRightPressed() const
{
    return m_keysPressed.contains(eng::Key::D) && m_keysPressed.at(eng::Key::D);
}

bool gme::GameSolo::isShootPressed() const
{
    return m_keysPressed.contains(eng::Key::Space) && m_keysPressed.at(eng::Key::Space);
}

void gme::GameSolo::updatePlayerSkin()
{
    auto &registry = getRegistry();
    auto *playerRect = registry.getComponent<ecs::Rect>(m_playerEntity);

    if (playerRect != nullptr)
    {
        const float skinPosY = static_cast<float>(m_skinIndex) * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }
}
