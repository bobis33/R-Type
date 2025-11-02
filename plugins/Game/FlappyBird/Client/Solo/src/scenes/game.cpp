#include <cmath>
#include <cstdlib>
#include <ctime>

#include "ECS/Component.hpp"
#include "FlappyBirdClientSolo/Scenes/Game.hpp"
#include "Utils/Common.hpp"
#include "Utils/HitboxUtils.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

enum class PipeType
{
    Small,
    Medium,
    Large
};

PipeType randomPipeType()
{
    const int r = rand() % 3;
    if (r == 0)
        return PipeType::Small;
    if (r == 1)
        return PipeType::Medium;
    return PipeType::Large;
}

float getGapSize(const PipeType type)
{
    switch (type)
    {
        case PipeType::Small:
            return 140.f;
        case PipeType::Medium:
            return 180.f;
        case PipeType::Large:
            return 220.f;
    }
    return 180.f;
}

gme::Game::Game(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                const std::shared_ptr<eng::IAudio> &audio, bool &showDebug, int skinIndex,
                const std::string &playerName)
    : AScene(assignedId), m_renderer(renderer), m_audio(audio), m_showDebug(showDebug), m_skinIndex(skinIndex),
      m_playerName(playerName)
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

            if (hitBox && transform)
            {
                bool isPipe = false;
                if (registry.hasComponent<ecs::Texture>(e))
                {
                    const auto *texture = registry.getComponent<ecs::Texture>(e);
                    if (texture && (texture->id.find("pipe") != std::string::npos))
                    {
                        isPipe = true;
                    }
                }
                
                if (!isPipe)
                {
                    renderer->createCircleShape({.name = "hitbox_" + std::to_string(e),
                                                 .radius = hitBox->radius,
                                                 .color = {.r = 255, .g = 0, .b = 0, .a = 100},
                                                 .x = transform->x + hitBox->offsetX,
                                                 .y = transform->y + hitBox->offsetY,
                                                 .outline_thickness = 1.0f,
                                                 .outline_color = {.r = 255, .g = 0, .b = 0, .a = 200}});
                }
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

    m_playerEntity = createPlayer(registry);

    m_playerNameEntity = registry.createEntity()
                            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                            .with<ecs::Transform>("transform_player_name", 200.0F, 160.0F, 0.F)
                            .with<ecs::Color>("color_player_name", utl::Config::Color::CYAN_ELECTRIC.r,
                                              utl::Config::Color::CYAN_ELECTRIC.g, utl::Config::Color::CYAN_ELECTRIC.b,
                                              utl::Config::Color::CYAN_ELECTRIC.a)
                            .with<ecs::Text>("id_player_name", m_playerName.empty() ? "Player" : m_playerName, 20U)
                            .build();

    auto [width, height] = m_renderer->getWindowSize();
    m_pipes.emplace_back(createPipePair(registry, width + 200.f, height / 2.f));
    m_pipes.emplace_back(createPipePair(registry, width + 600.f, height / 2.f + 50.f));
    m_pipes.emplace_back(createPipePair(registry, width + 1000.f, height / 2.f - 50.f));
    m_pipeScored = {false, false, false};

    m_scoreEntity = registry.createEntity()
                        .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                        .with<ecs::Transform>("transform_score", 50.0F, 50.0F, 0.F)
                        .with<ecs::Color>("color_score", utl::Config::Color::CYAN_ELECTRIC.r,
                                          utl::Config::Color::CYAN_ELECTRIC.g, utl::Config::Color::CYAN_ELECTRIC.b,
                                          utl::Config::Color::CYAN_ELECTRIC.a)
                        .with<ecs::Text>("id_score", std::string("Score: 0"), 32U)
                        .build();

    m_looseText =
        registry.createEntity()
            .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
            .with<ecs::Transform>("transform_loose", 0.0F, height / 2.0F, 0.F)
            .with<ecs::Color>("color_loose", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g,
                              utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
            .with<ecs::Text>("id_loose", std::string("You loose, press R to restart"), 24U)
            .build();
    m_looseSound = registry.createEntity()
                       .with<ecs::Audio>("audio" + std::to_string(m_looseSound), "assets/audio/flappy-die.ogg", 10.0F,
                                         false, false)
                       .build();
    m_flapSound = registry.createEntity()
                      .with<ecs::Audio>("audio" + std::to_string(m_flapSound), "assets/audio/flappy-flap.ogg", 10.0F,
                                        false, false)
                      .build();
}

void gme::Game::update(const float dt, const eng::WindowSize &size)
{
    auto &registry = AScene::getRegistry();
    auto *transformLooseText = registry.getComponent<ecs::Transform>(m_looseText);
    auto *soundLoose = registry.getComponent<ecs::Audio>(m_looseSound);

    transformLooseText->x = 90000.0F;

    if (m_gameOver)
    {
        transformLooseText->x = 100.0F;
        if (!m_gameOverShown)
        {
            soundLoose->play = true;
            m_gameOverShown = true;
        }
        return;
    }

    checkCollisions(registry);
    checkScore(registry);
    
    if (m_showDebug)
    {
        drawDebugRectangles(registry);
    }

    auto *transform = registry.getComponent<ecs::Transform>(m_playerEntity);
    auto *velocity = registry.getComponent<ecs::Velocity>(m_playerEntity);
    auto *playerNameTransform = registry.getComponent<ecs::Transform>(m_playerNameEntity);

    if (transform && velocity)
    {
        constexpr float gravity = 900.f;
        velocity->y += gravity * dt;
        
        // Update player name position to follow the player
        if (playerNameTransform)
        {
            playerNameTransform->x = transform->x;
            playerNameTransform->y = transform->y - 40.0F;
        }
        transform->y += velocity->y * dt;

        if (transform->y > size.height - 50)
        {
            transform->y = size.height - 50;
            velocity->y = 0.f;
            m_gameOver = true;
        }
        if (transform->y < 0)
        {
            transform->y = 0;
            velocity->y = 0.f;
        }

        m_renderer->setSpritePosition("player_texture" + std::to_string(m_playerEntity), transform->x, transform->y);
    }

    for (std::size_t i = 0; i < m_pipes.size(); ++i)
    {
        auto &[topPipe, bottomPipe] = m_pipes[i];
        auto *topTransform = registry.getComponent<ecs::Transform>(topPipe);
        auto *botTransform = registry.getComponent<ecs::Transform>(bottomPipe);
        const auto *topVel = registry.getComponent<ecs::Velocity>(topPipe);

        if (const auto *botVel = registry.getComponent<ecs::Velocity>(bottomPipe);
            topTransform && botTransform && topVel && botVel)
        {
            topTransform->x += topVel->x * dt;
            botTransform->x += botVel->x * dt;

            if (topTransform->x < -150)
            {
                topTransform->x = size.width + 200.f;
                botTransform->x = size.width + 200.f;

                const PipeType type = randomPipeType();
                const float gapSize = getGapSize(type);
                const float gapY = static_cast<float>(100 + rand() % (size.height - 100 - static_cast<int>(gapSize)));

                const float scale = static_cast<float>(size.width) / 1920.f;
                const float pipeHeight = 153.f * scale * 3.f;
                topTransform->y = gapY - pipeHeight / 2.f;
                botTransform->y = gapY + gapSize + pipeHeight / 2.f;

                if (i < m_pipeScored.size())
                {
                    m_pipeScored[i] = false;
                }
            }

            m_renderer->setSpritePosition("pipe_top_texture" + std::to_string(topPipe), topTransform->x,
                                          topTransform->y);
            m_renderer->setSpritePosition("pipe_bottom_texture" + std::to_string(bottomPipe), botTransform->x,
                                          botTransform->y);
        }
    }
}

void gme::Game::event(const eng::Event &event)
{
    auto &registry = AScene::getRegistry();
    auto *soundFlap = registry.getComponent<ecs::Audio>(m_flapSound);

    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Space && !m_gameOver)
            {
                soundFlap->play = true;
                if (auto *vel = registry.getComponent<ecs::Velocity>(m_playerEntity))
                    vel->y = -300.f;
            }
            else if (event.key == eng::Key::R && m_gameOver)
            {
                resetGame();
            }
            break;

        default:
            break;
    }
}

ecs::Entity gme::Game::createPlayer(ecs::Registry &registry)
{
    auto [offsetX, offsetY] = utl::calculateHitboxOffsets(utl::GameConfig::Player::SPRITE_WIDTH, utl::GameConfig::Player::SPRITE_HEIGHT, utl::GameConfig::Player::SCALE);
    const float skinPosY = m_skinIndex * utl::GameConfig::Player::SPRITE_HEIGHT;

    return registry.createEntity()
        .with<ecs::Transform>("player_transform", 200.0F, 200.0F, 0.F)
        .with<ecs::Velocity>("player_velocity", 0.F, 0.F)
        .with<ecs::Rect>("player_rect", 0.F, skinPosY, 33, 17)
        .with<ecs::Scale>("player_scale", 2.0F, 2.0F)
        .with<ecs::Texture>("player_texture", utl::Path::Texture::TEXTURE_PLAYER)
        .with<ecs::Player>("player", true)
        .with<ecs::Hitbox>("player_hitbox", 20.0F, offsetX, offsetY)
        .build();
}

std::pair<ecs::Entity, ecs::Entity> gme::Game::createPipePair(ecs::Registry &registry, float x, float /*gapY*/) const
{
    auto [width, height] = m_renderer->getWindowSize();
    const float scale = width / 1920.f;
    const float pipeOriginalWidth = 153.f;
    const float pipeOriginalHeight = 22.f;
    const float pipeScaledWidth = pipeOriginalWidth * scale * 3.f;
    const float pipeScaledHeight = pipeOriginalHeight * scale * 3.f;
    const float pipeRadius = pipeScaledWidth / 2.f * 1.2f;

    auto topPipe = registry.createEntity()
                       .with<ecs::Transform>("pipe_top_transform", x, 0.f, 0.f)
                       .with<ecs::Velocity>("pipe_top_velocity", -200.f * scale, 0.f)
                       .with<ecs::Scale>("pipe_top_scale", scale * 3.f, scale * 3.f)
                       .with<ecs::Texture>("pipe_top_texture", "assets/sprites/flappybird-pipe-xl.png")
                       .with<ecs::Hitbox>("pipe_top_hitbox", pipeRadius, 0.f, 0.f)
                       .build();

    m_renderer->setSpriteRotation("pipe_top_texture" + std::to_string(topPipe), -90.f);
    m_renderer->setSpriteOrigin("pipe_top_texture" + std::to_string(topPipe));

    auto bottomPipe = registry.createEntity()
                          .with<ecs::Transform>("pipe_bottom_transform", x, static_cast<float>(height), 0.f)
                          .with<ecs::Velocity>("pipe_bottom_velocity", -200.f * scale, 0.f)
                          .with<ecs::Scale>("pipe_bottom_scale", scale * 3.f, scale * 3.f)
                          .with<ecs::Texture>("pipe_bottom_texture", "assets/sprites/flappybird-pipe-xl.png")
                          .with<ecs::Hitbox>("pipe_bottom_hitbox", pipeRadius, 0.f, 0.f)
                          .build();

    m_renderer->setSpriteRotation("pipe_bottom_texture" + std::to_string(bottomPipe), 90.f);
    m_renderer->setSpriteOrigin("pipe_bottom_texture" + std::to_string(bottomPipe));

    return {topPipe, bottomPipe};
}

void gme::Game::resetGame()
{
    auto &registry = AScene::getRegistry();
    auto *playerTransform = registry.getComponent<ecs::Transform>(m_playerEntity);

    if (auto *playerVel = registry.getComponent<ecs::Velocity>(m_playerEntity);
        (playerTransform != nullptr) && playerVel)
    {
        playerTransform->x = 200.f;
        playerTransform->y = 200.f;
        playerVel->y = 0.f;
    }

    auto [width, height] = m_renderer->getWindowSize();
    const float scale = width / 1920.f;
    float startX = width + 200.f;

    for (std::size_t i = 0; i < m_pipes.size(); ++i)
    {
        auto &[topPipe, bottomPipe] = m_pipes[i];
        auto *topTransform = registry.getComponent<ecs::Transform>(topPipe);
        if (auto *botTransform = registry.getComponent<ecs::Transform>(bottomPipe); topTransform && botTransform)
        {
            topTransform->x = startX;
            botTransform->x = startX;
            const PipeType type = randomPipeType();
            const float gapSize = getGapSize(type);
            const float gapY = static_cast<float>(100 + rand() % (height - 100 - static_cast<int>(gapSize)));

            const float pipeHeight = 153.f * scale * 3.f;
            topTransform->y = gapY - pipeHeight / 2.f;
            botTransform->y = gapY + gapSize + pipeHeight / 2.f;
        }
        startX += 400.f;
        if (i < m_pipeScored.size())
        {
            m_pipeScored[i] = false;
        }
    }

    m_gameOver = false;
    m_gameOverShown = false;
    m_score = 0;
    if (auto *scoreText = registry.getComponent<ecs::Text>(m_scoreEntity))
    {
        scoreText->content = "Score: 0";
        m_renderer->setTextContent("id_score", "Score: 0");
    }
}

bool gme::Game::checkCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2) const
{
    const float dx = x1 - x2;
    const float dy = y1 - y2;
    const float distance = std::sqrt(dx * dx + dy * dy);
    const float combinedRadius = r1 + r2;

    return distance < combinedRadius;
}

bool gme::Game::checkCircleRectCollision(float circleX, float circleY, float circleR, float rectX, float rectY, float rectW, float rectH) const
{
    const float closestX = std::max(rectX, std::min(circleX, rectX + rectW));
    const float closestY = std::max(rectY, std::min(circleY, rectY + rectH));
    
    const float dx = circleX - closestX;
    const float dy = circleY - closestY;
    const float distanceSquared = dx * dx + dy * dy;
    
    return distanceSquared < circleR * circleR;
}

void gme::Game::checkCollisions(ecs::Registry &registry)
{
    if (m_gameOver)
        return;

    auto *playerTransform = registry.getComponent<ecs::Transform>(m_playerEntity);
    auto *playerHitbox = registry.getComponent<ecs::Hitbox>(m_playerEntity);

    if (!playerTransform || !playerHitbox)
        return;

    const float playerX = playerTransform->x + playerHitbox->offsetX;
    const float playerY = playerTransform->y + playerHitbox->offsetY;
    const float playerRadius = playerHitbox->radius;

    auto [width, height] = m_renderer->getWindowSize();
    const float scale = width / 1920.f;
    const float pipeOriginalWidth = 153.f;
    const float pipeOriginalHeight = 22.f;
    const float pipeScaledWidth = pipeOriginalWidth * scale * 3.f;
    const float pipeScaledHeight = pipeOriginalHeight * scale * 3.f;

    for (const auto &[topPipe, bottomPipe] : m_pipes)
    {
        auto *topTransform = registry.getComponent<ecs::Transform>(topPipe);
        auto *botTransform = registry.getComponent<ecs::Transform>(bottomPipe);

        if (!topTransform || !botTransform)
            continue;

        const float topRectX = topTransform->x - pipeScaledHeight / 2.f;
        const float topRectY = topTransform->y - pipeScaledWidth / 2.f;
        const float topRectW = pipeScaledHeight;
        const float topRectH = pipeScaledWidth;

        const float botRectX = botTransform->x - pipeScaledHeight / 2.f;
        const float botRectY = botTransform->y - pipeScaledWidth / 2.f;
        const float botRectW = pipeScaledHeight;
        const float botRectH = pipeScaledWidth;

        if (checkCircleRectCollision(playerX, playerY, playerRadius, topRectX, topRectY, topRectW, topRectH) ||
            checkCircleRectCollision(playerX, playerY, playerRadius, botRectX, botRectY, botRectW, botRectH))
        {
            m_gameOver = true;
            return;
        }
    }
}

void gme::Game::checkScore(ecs::Registry &registry)
{
    if (m_gameOver)
        return;

    auto *playerTransform = registry.getComponent<ecs::Transform>(m_playerEntity);
    if (!playerTransform)
        return;

    const float playerX = playerTransform->x;

    for (std::size_t i = 0; i < m_pipes.size(); ++i)
    {
        if (i >= m_pipeScored.size())
            continue;

        if (m_pipeScored[i])
            continue;

        auto &[topPipe, bottomPipe] = m_pipes[i];
        auto *topTransform = registry.getComponent<ecs::Transform>(topPipe);

        if (!topTransform)
            continue;

        if (playerX > topTransform->x + 100.f)
        {
            m_pipeScored[i] = true;
            m_score++;
            if (auto *scoreText = registry.getComponent<ecs::Text>(m_scoreEntity))
            {
                scoreText->content = "Score: " + std::to_string(m_score);
                m_renderer->setTextContent("id_score", "Score: " + std::to_string(m_score));
            }
        }
    }
}

void gme::Game::drawDebugRectangles(ecs::Registry &registry) const
{
    auto [width, height] = m_renderer->getWindowSize();
    const float scale = width / 1920.f;
    const float pipeOriginalWidth = 153.f;
    const float pipeOriginalHeight = 22.f;
    const float pipeScaledWidth = pipeOriginalWidth * scale * 3.f;
    const float pipeScaledHeight = pipeOriginalHeight * scale * 3.f;
    
    const eng::Color debugColor = {.r = 0, .g = 255, .b = 0, .a = 200};
    
    for (const auto &[topPipe, bottomPipe] : m_pipes)
    {
        auto *topTransform = registry.getComponent<ecs::Transform>(topPipe);
        auto *botTransform = registry.getComponent<ecs::Transform>(bottomPipe);
        
        if (!topTransform || !botTransform)
            continue;
        
        const float topRectX = topTransform->x - pipeScaledHeight / 2.f;
        const float topRectY = topTransform->y - pipeScaledWidth / 2.f;
        const float topRectW = pipeScaledHeight;
        const float topRectH = pipeScaledWidth;
        
        const float botRectX = botTransform->x - pipeScaledHeight / 2.f;
        const float botRectY = botTransform->y - pipeScaledWidth / 2.f;
        const float botRectW = pipeScaledHeight;
        const float botRectH = pipeScaledWidth;
        
        const float step = 2.f;
        for (float x = topRectX; x <= topRectX + topRectW; x += step)
        {
            m_renderer->drawPoint(x, topRectY, debugColor);
            m_renderer->drawPoint(x, topRectY + topRectH, debugColor);
        }
        for (float y = topRectY; y <= topRectY + topRectH; y += step)
        {
            m_renderer->drawPoint(topRectX, y, debugColor);
            m_renderer->drawPoint(topRectX + topRectW, y, debugColor);
        }
        
        for (float x = botRectX; x <= botRectX + botRectW; x += step)
        {
            m_renderer->drawPoint(x, botRectY, debugColor);
            m_renderer->drawPoint(x, botRectY + botRectH, debugColor);
        }
        for (float y = botRectY; y <= botRectY + botRectH; y += step)
        {
            m_renderer->drawPoint(botRectX, y, debugColor);
            m_renderer->drawPoint(botRectX + botRectW, y, debugColor);
        }
    }
}
