#include "Client/Systems/HUD.hpp"
#include "Client/GameConfig.hpp"

void cli::HUDSystem::createScoreHUD(ecs::Registry &registry, float x, float y)
{
    m_scoreBgEntity = registry.createEntity()
                          .with<ecs::Transform>("score_bg_transform", x, y, 0.0f)
                          .with<ecs::Scale>("score_bg_scale", 3.0f, 2.0f)
                          .with<ecs::Texture>("score_bg_texture", Path::Texture::TEXTURE_SCORE_COUNTER_BG)
                          .with<ecs::Layer>("score_bg_layer", 1)
                          .build();

    m_scoreDigitEntities.clear();
    for (int i = 0; i < 6; ++i)
    {
        auto digitEntity = registry.createEntity()
                               .with<ecs::Transform>("score_digit_transform", x + 140.0f + i * 20.0f, y + 4.0f, 0.0f)
                               .with<ecs::Scale>("score_digit_scale", 1.0f, 1.0f)
                               .with<ecs::Texture>("score_digit_texture", Path::Texture::TEXTURE_SCORE_DIGIT_0)
                               .with<ecs::Layer>("score_digit_layer", 2)
                               .build();
        m_scoreDigitEntities.push_back(digitEntity);
    }
}

void cli::HUDSystem::updateScore(ecs::Registry &registry, int newScore)
{
    int digits[6];
    for (int i = 5; i >= 0; --i)
    {
        digits[i] = newScore % 10;
        newScore /= 10;
    }

    static const char *digitPaths[10] = {Path::Texture::TEXTURE_SCORE_DIGIT_0, Path::Texture::TEXTURE_SCORE_DIGIT_1,
                                         Path::Texture::TEXTURE_SCORE_DIGIT_2, Path::Texture::TEXTURE_SCORE_DIGIT_3,
                                         Path::Texture::TEXTURE_SCORE_DIGIT_4, Path::Texture::TEXTURE_SCORE_DIGIT_5,
                                         Path::Texture::TEXTURE_SCORE_DIGIT_6, Path::Texture::TEXTURE_SCORE_DIGIT_7,
                                         Path::Texture::TEXTURE_SCORE_DIGIT_8, Path::Texture::TEXTURE_SCORE_DIGIT_9};

    for (int i = 0; i < 6 && i < m_scoreDigitEntities.size(); ++i)
    {
        auto *digitTexture = registry.getComponent<ecs::Texture>(m_scoreDigitEntities[i]);
        if (digitTexture)
        {
            digitTexture->path = digitPaths[digits[i]];
        }

        auto *digitTransform = registry.getComponent<ecs::Transform>(m_scoreDigitEntities[i]);
        auto *digitScale = registry.getComponent<ecs::Scale>(m_scoreDigitEntities[i]);
        if (digitTransform && digitScale)
        {
            digitTransform->x = 10.0f + 130.0f + i * 20.0f;
            digitTransform->y = 10.0f + 4.0f;
            digitScale->x = 1.5f;
            digitScale->y = 1.5f;
        }
    }
}

void cli::HUDSystem::update(ecs::Registry &registry, float /* dt */)
{
    int currentScore = 0;
    for (auto &[scoreEntity, score] : registry.getAll<ecs::Score>())
    {
        currentScore = score.value;
        break;
    }

    updateScore(registry, currentScore);
}
