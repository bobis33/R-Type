#include "RTypeClientMulti/Managers/StageManager.hpp"
#include "ECS/Component.hpp"
#include "Utils/Common.hpp"

void gme::StageManager::update(ecs::Registry &registry, const float dt, const eng::WindowSize &size)
{
    m_stageTimer += dt;
    
    if (!m_stageSpawned && m_stageTimer >= STAGE_SPAWN_DELAY)
    {
        spawnStage(registry, static_cast<int>(size.width));
        m_stageSpawned = true;
    }
    
    switch (m_stageState)
    {
        case StageState::SCROLLING:
            if (m_stageSpawned && m_stageTimer >= STAGE_DURATION)
            {
                startBossPhase();
            }
            break;
            
        case StageState::BOSS_PHASE:
            m_bossTimer += dt;
            if (m_bossTimer >= BOSS_PHASE_DURATION)
            {
                completeStage();
            }
            break;
            
        case StageState::COMPLETED:
            break;
    }
}

void gme::StageManager::startBossPhase()
{
    m_stageState = StageState::BOSS_PHASE;
    m_bossTimer = 0.0f;
}

void gme::StageManager::completeStage()
{
    m_stageState = StageState::COMPLETED;
}

float gme::StageManager::getStageProgress() const
{
    if (!m_stageSpawned)
        return 0.0f;
    
    switch (m_stageState)
    {
        case StageState::SCROLLING:
            return std::min(1.0f, m_stageTimer / STAGE_DURATION);
        case StageState::BOSS_PHASE:
            return 1.0f;
        case StageState::COMPLETED:
            return 1.0f;
        default:
            return 0.0f;
    }
}

float gme::StageManager::getBossPhaseProgress() const
{
    if (m_stageState != StageState::BOSS_PHASE)
        return 0.0f;
    
    return std::min(1.0f, m_bossTimer / BOSS_PHASE_DURATION);
}

std::string gme::StageManager::getStageStatusText() const
{
    switch (m_stageState)
    {
        case StageState::SCROLLING:
            return "Stage 1 - Scrolling";
        case StageState::BOSS_PHASE:
            return "Stage 1 - Boss Phase";
        case StageState::COMPLETED:
            return "Stage 1 - Completed";
        default:
            return "Stage 1 - Loading";
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

