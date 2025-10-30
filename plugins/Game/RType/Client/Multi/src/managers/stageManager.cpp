#include "RTypeClientMulti/Managers/StageManager.hpp"
#include "ECS/Component.hpp"
#include "Utils/Common.hpp"

void gme::StageManager::update(ecs::Registry &registry, const float dt, const eng::WindowSize &size)
{
    m_stageTimer += dt;
    if (!m_stageSpawned && m_stageTimer >= STAGE_SPAWN_DELAY)
    {
        spawnStage(registry, size.width);
        m_stageSpawned = true;
    }
}

void gme::StageManager::stopScrolling(ecs::Registry &registry)
{
    for (auto &[entity, scrolling] : registry.getAll<ecs::Scrolling>())
    {
        scrolling.speed_x = 0.0f;
    }
}

void gme::StageManager::spawnStage(ecs::Registry &registry, const int screenWidth)
{
    (void) registry.createEntity()
                      .with<ecs::Transform>("floor1_transform", 0.F, 0.F, 0.F)
                      .with<ecs::Scale>("floor1_scale", 1.F, 1.F)
                      .with<ecs::Texture>("floor1_texture", utl::Path::Texture::TEXTURE_STAGE1_FLOOR)
                      .with<ecs::Scrolling>("floor1_scroll", SCROLL_SPEED, ORIGINAL_WIDTH, SPRITE_HEIGHT, true)
                      .with<ecs::Floor>("floor1_tag")
                      .build();

    (void) registry.createEntity()
                      .with<ecs::Transform>("floor2_transform", static_cast<float>(screenWidth), 0.F, 0.F)
                      .with<ecs::Scale>("floor2_scale", 1.F, 1.F)
                      .with<ecs::Texture>("floor2_texture", utl::Path::Texture::TEXTURE_STAGE1_FLOOR)
                      .with<ecs::Scrolling>("floor2_scroll", SCROLL_SPEED, ORIGINAL_WIDTH, SPRITE_HEIGHT, true)
                      .with<ecs::Floor>("floor2_tag")
                      .build();

    (void) registry.createEntity()
                     .with<ecs::Transform>("ceil1_transform", 0.F, 0.F, 0.F)
                     .with<ecs::Scale>("ceil1_scale", 1.F, 1.F)
                     .with<ecs::Texture>("ceil1_texture", utl::Path::Texture::TEXTURE_STAGE1_CEILING)
                     .with<ecs::Scrolling>("ceil1_scroll", SCROLL_SPEED, ORIGINAL_WIDTH, SPRITE_HEIGHT, true)
                     .with<ecs::Ceiling>("ceil1_tag")
                     .build();

    (void) registry.createEntity()
                     .with<ecs::Transform>("ceil2_transform", static_cast<float>(screenWidth), 0.F, 0.F)
                     .with<ecs::Scale>("ceil2_scale", 1.F, 1.F)
                     .with<ecs::Texture>("ceil2_texture", utl::Path::Texture::TEXTURE_STAGE1_CEILING)
                     .with<ecs::Scrolling>("ceil2_scroll", SCROLL_SPEED, ORIGINAL_WIDTH, SPRITE_HEIGHT, true)
                     .with<ecs::Ceiling>("ceil2_tag")
                     .build();
}

