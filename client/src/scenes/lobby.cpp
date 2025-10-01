#include "Client/Scenes/Lobby.hpp"
#include "Client/Common.hpp"
#include "Client/GameConfig.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"

static constexpr eng::Color WHITE = {.r = 255U, .g = 255U, .b = 255U, .a = 255U};

cli::Lobby::Lobby(const std::shared_ptr<eng::IRenderer> &renderer, const std::shared_ptr<eng::IAudio> &audio)
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
    registry.createEntity()
        .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
        .with<ecs::Transform>("transform_title", 10.F, 10.F, 0.F)
        .with<ecs::Color>("color_title", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
        .with<ecs::Text>("id", std::string("RType Client"), 50U)
        .build();
    m_fpsEntity = registry.createEntity()
                      .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                      .with<ecs::Transform>("transform_fps", 10.F, 70.F, 0.F)
                      .with<ecs::Color>("color_fps", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
                      .with<ecs::Text>("id_text", std::string("FPS: 0"), 20U)
                      .build();

    // Compteur d'ennemis
    m_enemyCounterEntity = registry.createEntity()
                              .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                              .with<ecs::Transform>("transform_enemy_counter", 10.F, 100.F, 0.F)
                              .with<ecs::Color>("color_enemy_counter", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
                              .with<ecs::Text>("id_enemy_counter", std::string("Enemies: 0"), 20U)
                              .build();

    // Compteur d'astéroïdes
    m_asteroidCounterEntity = registry.createEntity()
                                 .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                                 .with<ecs::Transform>("transform_asteroid_counter", 10.F, 130.F, 0.F)
                                 .with<ecs::Color>("color_asteroid_counter", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
                                 .with<ecs::Text>("id_asteroid_counter", std::string("Asteroids: 0"), 20U)
                                 .build();


    m_playerEntity = registry.createEntity()
                         .with<ecs::Transform>("player_transform", 200.F, 100.F, 0.F)
                         .with<ecs::Velocity>("player_velocity", 0.F, 0.F)
                         .with<ecs::Rect>("player_rect", 0.F, 0.F, GameConfig::Player::SPRITE_WIDTH, GameConfig::Player::SPRITE_HEIGHT)
                         .with<ecs::Scale>("player_scale", GameConfig::Player::SCALE, GameConfig::Player::SCALE)
                         .with<ecs::Texture>("player_texture", Path::Texture::TEXTURE_PLAYER)
                         .with<ecs::Player>("player", true)
                         .with<ecs::BeamCharge>("beam_charge", 0.0f, GameConfig::Beam::MAX_CHARGE)
                         .with<ecs::Hitbox>("player_hitbox", GameConfig::Hitbox::PLAYER_RADIUS)
                         .build();

        // La barre de Beam sera affichée directement au-dessus du joueur
        // Pas besoin d'une entité séparée
    // Créer des étoiles pour l'effet de parallax simple
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    
    // Étoiles lointaines (lentes)
    for (int i = 0; i < 50; ++i) {
        registry.createEntity()
            .with<ecs::Pixel>("star_far")
            .with<ecs::Transform>("star_far_transform", 
                static_cast<float>(std::rand() % screenWidth), 
                static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_far_color", 255, 255, 255, 100) // Blanc transparent
            .with<ecs::Velocity>("star_far_vel", -20.0f, 0.0f)
            .build();
    }
    
    // Étoiles moyennes
    for (int i = 0; i < 30; ++i) {
        registry.createEntity()
            .with<ecs::Pixel>("star_mid")
            .with<ecs::Transform>("star_mid_transform", 
                static_cast<float>(std::rand() % screenWidth), 
                static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_mid_color", 200, 200, 255, 150) // Bleu clair
            .with<ecs::Velocity>("star_mid_vel", -40.0f, 0.0f)
            .build();
    }
    
    // Étoiles proches (rapides)
    for (int i = 0; i < 20; ++i) {
        registry.createEntity()
            .with<ecs::Pixel>("star_near")
            .with<ecs::Transform>("star_near_transform", 
                static_cast<float>(std::rand() % screenWidth), 
                static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_near_color", 255, 255, 200, 200) // Jaune clair
            .with<ecs::Velocity>("star_near_vel", -80.0f, 0.0f)
            .build();
    }
    
    // Étoiles filantes
    for (int i = 0; i < 10; ++i) {
        registry.createEntity()
            .with<ecs::Pixel>("star_shooting")
            .with<ecs::Transform>("star_shooting_transform", 
                static_cast<float>(std::rand() % screenWidth), 
                static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("star_shooting_color", 255, 100, 100, 255) // Rouge
            .with<ecs::Velocity>("star_shooting_vel", -120.0f, static_cast<float>((std::rand() % 20) - 10))
            .build();
    }

    // Planètes lointaines (très lentes)
    for (int i = 0; i < 5; ++i) {
        registry.createEntity()
            .with<ecs::Pixel>("planet_far")
            .with<ecs::Transform>("planet_far_transform", 
                static_cast<float>(std::rand() % screenWidth), 
                static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("planet_far_color", 100, 50, 150, 80) // Violet foncé
            .with<ecs::Velocity>("planet_far_vel", -5.0f, 0.0f)
            .build();
    }

    // Nébuleuses (très lentes, grandes)
    for (int i = 0; i < 3; ++i) {
        registry.createEntity()
            .with<ecs::Pixel>("nebula")
            .with<ecs::Transform>("nebula_transform", 
                static_cast<float>(std::rand() % screenWidth), 
                static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("nebula_color", 50, 100, 200, 60) // Bleu transparent
            .with<ecs::Velocity>("nebula_vel", -8.0f, 0.0f)
            .build();
    }

    // Comètes (mouvement diagonal)
    for (int i = 0; i < 8; ++i) {
        registry.createEntity()
            .with<ecs::Pixel>("comet")
            .with<ecs::Transform>("comet_transform", 
                static_cast<float>(std::rand() % screenWidth), 
                static_cast<float>(std::rand() % screenHeight), 0.0f)
            .with<ecs::Color>("comet_color", 200, 255, 200, 180) // Vert clair
            .with<ecs::Velocity>("comet_vel", -60.0f, static_cast<float>((std::rand() % 40) - 20))
            .build();
    }

}

    void cli::Lobby::update(const float dt, const eng::WindowSize &size)
    {
        auto &reg = getRegistry();
        auto *playerTransform = reg.getComponent<ecs::Transform>(m_playerEntity);
        auto *playerVelocity = reg.getComponent<ecs::Velocity>(m_playerEntity);


        m_weaponSystem.update(reg, dt, m_keysPressed[eng::Key::Space]);
    // Mise à jour des étoiles simples
    for (auto &[entity, pixel] : reg.getAll<ecs::Pixel>())
    {
        if (auto *transform = reg.getComponent<ecs::Transform>(entity))
        {
            if (auto *velocity = reg.getComponent<ecs::Velocity>(entity))
            {
                // Mise à jour de la position
                transform->x += velocity->x * dt;
                transform->y += velocity->y * dt;

                // Réinitialiser si l'étoile sort de l'écran
                if (transform->x < -10.0f || transform->x > size.width + 10.0f || 
                    transform->y < -10.0f || transform->y > size.height + 10.0f)
                {
                    transform->x = static_cast<float>(size.width + std::rand() % 200);
                    transform->y = static_cast<float>(std::rand() % size.height);
                }
            }
        }
    }
    if (auto *fpsText = reg.getComponent<ecs::Text>(m_fpsEntity))
    {
        fpsText->content = "FPS: " + std::to_string(static_cast<int>(1 / dt));
    }

    // Mettre à jour le compteur d'ennemis
    if (auto *enemyCounterText = reg.getComponent<ecs::Text>(m_enemyCounterEntity))
    {
        int enemyCount = 0;
        for (auto &[entity, enemy] : reg.getAll<ecs::Enemy>())
        {
            enemyCount++;
        }
        enemyCounterText->content = "Enemies: " + std::to_string(enemyCount);
    }

    // Mettre à jour le compteur d'astéroïdes
    if (auto *asteroidCounterText = reg.getComponent<ecs::Text>(m_asteroidCounterEntity))
    {
        int asteroidCount = 0;
        for (auto &[entity, asteroid] : reg.getAll<ecs::Asteroid>())
        {
            asteroidCount++;
        }
        asteroidCounterText->content = "Asteroids: " + std::to_string(asteroidCount);
    }
    float speed = GameConfig::Player::SPEED;
    float diagonal_speed = speed * GameConfig::Player::DIAGONAL_SPEED_MULTIPLIER;
    
    playerVelocity->x = 0.0f;
    playerVelocity->y = 0.0f;
    
    bool up = m_keysPressed[eng::Key::Up];
    bool down = m_keysPressed[eng::Key::Down];
    bool left = m_keysPressed[eng::Key::Left];
    bool right = m_keysPressed[eng::Key::Right];
    
    if (up && right)
    {
        playerVelocity->x = diagonal_speed;
        playerVelocity->y = -diagonal_speed;
    }
    else if (up && left)
    {
        playerVelocity->x = -diagonal_speed;
        playerVelocity->y = -diagonal_speed;
    }
    else if (down && right)
    {
        playerVelocity->x = diagonal_speed;
        playerVelocity->y = diagonal_speed;
    }
    else if (down && left)
    {
        playerVelocity->x = -diagonal_speed;
        playerVelocity->y = diagonal_speed;
    }
    else
    {
        if (up) playerVelocity->y = -speed;
        if (down) playerVelocity->y = speed;
        if (left) playerVelocity->x = -speed;
        if (right) playerVelocity->x = speed;
    }
    
    playerTransform->x += playerVelocity->x * dt;
    playerTransform->y += playerVelocity->y * dt;
    playerTransform->x = std::max(playerTransform->x, 0.F);
    playerTransform->y = std::max(playerTransform->y, 0.F);
    playerTransform->x =
        std::min(playerTransform->x, static_cast<float>(size.width) - GameConfig::Player::SPRITE_WIDTH * GameConfig::Player::SCALE);
    playerTransform->y =
        std::min(playerTransform->y, static_cast<float>(size.height) - GameConfig::Player::SPRITE_HEIGHT * GameConfig::Player::SCALE);
}


void cli::Lobby::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Up)
                m_keysPressed[eng::Key::Up] = true;
            if (event.key == eng::Key::Down)
                m_keysPressed[eng::Key::Down] = true;
            if (event.key == eng::Key::Left)
                m_keysPressed[eng::Key::Left] = true;
            if (event.key == eng::Key::Right)
                m_keysPressed[eng::Key::Right] = true;
            if (event.key == eng::Key::Space)
                m_keysPressed[eng::Key::Space] = true;
            break;

        case eng::EventType::KeyReleased:
            if (event.key == eng::Key::Up)
                m_keysPressed[eng::Key::Up] = false;
            if (event.key == eng::Key::Down)
                m_keysPressed[eng::Key::Down] = false;
            if (event.key == eng::Key::Left)
                m_keysPressed[eng::Key::Left] = false;
            if (event.key == eng::Key::Right)
                m_keysPressed[eng::Key::Right] = false;
            if (event.key == eng::Key::Space)
                m_keysPressed[eng::Key::Space] = false;
            break;

        default:
            break;
    }
}
