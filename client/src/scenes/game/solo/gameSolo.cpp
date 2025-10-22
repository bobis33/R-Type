#include "Client/Scenes/game/solo/GameSolo.hpp"
#include "Client/Client.hpp"
#include "Client/Common.hpp"
#include "Client/GameConfig.hpp"
#include "Client/Managers/StageManager.hpp"
#include "Client/Systems/HUD.hpp"
#include "Client/Systems/PlayerController.hpp"
#include "Client/Systems/Starfield.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"

cli::GameSolo::GameSolo(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio,
                        const AppConfig &appConfig)
    : m_audio(audio), m_appConfig(appConfig)
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

    registry.createEntity().with<ecs::Audio>("id_audio", Path::Audio::AUDIO_TITLE, 5.F, true, true).build();
    registry.createEntity().with<ecs::Score>("score", 0).build();

    m_hudSystem = std::make_unique<HUDSystem>(renderer);
    m_starfieldSystem = std::make_unique<StarfieldSystem>(renderer);
    m_playerController = std::make_unique<PlayerController>(renderer);
    m_stageManager = std::make_unique<StageManager>();

    m_playerEntity = m_playerController->createPlayer(registry, 200.F, 100.F);
    m_hudSystem->createScoreHUD(registry, 10.0f, 10.0f);
}

void cli::GameSolo::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();
    auto &audios = reg.getAll<ecs::Audio>();

    for (auto &audio : audios)
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

void cli::GameSolo::event(const eng::Event &event)
{
    auto &reg = getRegistry();
    m_playerController->handleInput(reg, event);
}

bool cli::GameSolo::isUpPressed() const
{
    switch (m_appConfig.controlScheme)
    {
        case 0:
            return m_keysPressed.count(eng::Key::Z) && m_keysPressed.at(eng::Key::Z);
        case 1:
            return m_keysPressed.count(eng::Key::W) && m_keysPressed.at(eng::Key::W);
        default:
            return m_keysPressed.count(eng::Key::Up) && m_keysPressed.at(eng::Key::Up);
    }
}

bool cli::GameSolo::isDownPressed() const
{
    switch (m_appConfig.controlScheme)
    {
        case 0:
            return m_keysPressed.count(eng::Key::S) && m_keysPressed.at(eng::Key::S);
        case 1:
            return m_keysPressed.count(eng::Key::S) && m_keysPressed.at(eng::Key::S);
        default:
            return m_keysPressed.count(eng::Key::Down) && m_keysPressed.at(eng::Key::Down);
    }
}

bool cli::GameSolo::isLeftPressed() const
{
    switch (m_appConfig.controlScheme)
    {
        case 0:
            return m_keysPressed.count(eng::Key::Q) && m_keysPressed.at(eng::Key::Q);
        case 1:
            return m_keysPressed.count(eng::Key::A) && m_keysPressed.at(eng::Key::A);
        default:
            return m_keysPressed.count(eng::Key::Left) && m_keysPressed.at(eng::Key::Left);
    }
}

bool cli::GameSolo::isRightPressed() const
{
    switch (m_appConfig.controlScheme)
    {
        case 0:
            return m_keysPressed.count(eng::Key::D) && m_keysPressed.at(eng::Key::D);
        case 1:
            return m_keysPressed.count(eng::Key::D) && m_keysPressed.at(eng::Key::D);
        default:
            return m_keysPressed.count(eng::Key::Right) && m_keysPressed.at(eng::Key::Right);
    }
}

bool cli::GameSolo::isShootPressed() const
{
    return m_keysPressed.count(eng::Key::Space) && m_keysPressed.at(eng::Key::Space);
}

void cli::GameSolo::updatePlayerSkin()
{
    auto &registry = getRegistry();
    auto *playerRect = registry.getComponent<ecs::Rect>(m_playerEntity);

    if (playerRect)
    {
        float skinPosY = static_cast<float>(m_appConfig.skinIndex) * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }
}
