///
/// @file WeaponSystem.cpp
/// @brief Implementation of WeaponSystem
/// @namespace cli
///

#include "Client/Systems/Weapon.hpp"
#include "Client/Common.hpp"
#include "Client/ProjectileManager.hpp"

namespace cli
{
    void WeaponSystem::update(ecs::Registry &registry, float dt, bool spacePressed)
    {
        using namespace GameConfig::Projectile;
        using namespace GameConfig::Beam;

        // Update cooldowns
        if (m_fireCooldown > 0.0f)
        {
            m_fireCooldown -= dt;
        }

        // Get player entity and position
        auto playerEntities = registry.getAll<ecs::Player>();
        if (playerEntities.empty())
            return;

        auto &[playerEntity, player] = *playerEntities.begin();
        auto *transform = registry.getComponent<ecs::Transform>(playerEntity);
        auto *beamCharge = registry.getComponent<ecs::BeamCharge>(playerEntity);
        if (!transform || !beamCharge)
            return;

        float projectileX = transform->x + GameConfig::Player::SPRITE_WIDTH;
        float projectileY = transform->y + GameConfig::Player::SPRITE_HEIGHT / 2.0f;

        if (spacePressed)
        {
            // Commencer le chargement si ce n'était pas déjà le cas
            if (!m_isCharging)
            {
                m_isCharging = true;
            }

            // Charger la barre de Beam
            beamCharge->current_charge += CHARGE_RATE * dt;
            if (beamCharge->current_charge > beamCharge->max_charge)
                beamCharge->current_charge = beamCharge->max_charge;

            // Afficher l'animation de chargement si on charge
            if (beamCharge->current_charge < beamCharge->max_charge)
            {
                showLoadingAnimation(registry, playerEntity, transform);
            }
            else
            {
                hideLoadingAnimation(registry, playerEntity);
            }

            // PENDANT LE CHARGEMENT : NE RIEN TIRER DU TOUT
            return; // Sortir de la fonction sans rien tirer
        }
        else
        {
            // Si on était en train de charger et qu'on relâche espace
            if (m_isCharging)
            {
                m_isCharging = false;

                // Cacher l'animation de chargement
                hideLoadingAnimation(registry, playerEntity);

                // Quand on relâche espace, vérifier si on peut tirer
                if (m_fireCooldown <= 0.0f)
                {
                    // Si on a au moins 50% de charge, tirer un supercharged
                    float chargeThreshold = beamCharge->max_charge * 0.5f; // 50% minimum
                    if (beamCharge->current_charge >= chargeThreshold)
                    {
                        if (tryFireSupercharged(registry, projectileX, projectileY))
                        {
                            beamCharge->current_charge = 0.0f; // Consommer toute la charge
                            m_fireCooldown = Supercharged::FIRE_COOLDOWN;
                        }
                    }
                    else
                    {
                        // Si pas assez de charge, tirer un basic
                        tryFireBasic(registry, projectileX, projectileY);
                    }
                }
            }
        }
    }

    void WeaponSystem::reset()
    {
        m_fireCooldown = 0.0f;
        m_isCharging = false;
    }

    bool WeaponSystem::tryFireBasic(ecs::Registry &registry, float x, float y)
    {
        using namespace GameConfig::Projectile;

        if (m_fireCooldown > 0.0f)
            return false;

        ProjectileManager::createBasicProjectile(registry, x, y, Basic::SPEED, 0.0f);
        m_fireCooldown = Basic::FIRE_COOLDOWN;
        return true;
    }

    bool WeaponSystem::tryFireSupercharged(ecs::Registry &registry, float x, float y)
    {
        using namespace GameConfig::Projectile;

        ProjectileManager::createSuperchargedProjectile(registry, x, y, Supercharged::SPEED, 0.0f);
        return true;
    }

    void WeaponSystem::showLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity,
                                            const ecs::Transform *playerTransform)
    {
        using namespace GameConfig::LoadingAnimation;

        // Chercher s'il y a déjà une animation de chargement
        auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
        for (auto &[entity, animation] : loadingEntities)
        {
            auto *loadingTransform = registry.getComponent<ecs::Transform>(entity);
            if (loadingTransform)
            {
                // Mettre à jour la position
                loadingTransform->x = playerTransform->x + OFFSET_X;
                loadingTransform->y = playerTransform->y + OFFSET_Y;
                return; // Animation déjà présente
            }
        }

        // Créer une nouvelle animation de chargement
        auto loadingEntity =
            registry.createEntity()
                .with<ecs::Transform>("loading_transform", playerTransform->x + OFFSET_X, playerTransform->y + OFFSET_Y,
                                      0.0f)
                .with<ecs::Rect>("loading_rect", 0.0f, 0.0f, static_cast<int>(SPRITE_WIDTH),
                                 static_cast<int>(SPRITE_HEIGHT))
                .with<ecs::Scale>("loading_scale", 1.0f, 1.0f)
                .with<ecs::Texture>("loading_texture", Path::Texture::TEXTURE_SHOOT_LOADING)
                .with<ecs::LoadingAnimation>("loading_animation", 0, ANIMATION_FRAMES, ANIMATION_DURATION, 0.0f,
                                             SPRITE_WIDTH, SPRITE_HEIGHT, ANIMATION_FRAMES)
                .build();
    }

    void WeaponSystem::hideLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity)
    {
        // Supprimer toutes les animations de chargement
        auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
        std::vector<ecs::Entity> toRemove;

        for (auto &[entity, animation] : loadingEntities)
        {
            toRemove.push_back(entity);
        }

        for (auto entity : toRemove)
        {
            if (registry.hasComponent<ecs::Transform>(entity))
                registry.removeComponent<ecs::Transform>(entity);
            if (registry.hasComponent<ecs::Rect>(entity))
                registry.removeComponent<ecs::Rect>(entity);
            if (registry.hasComponent<ecs::Scale>(entity))
                registry.removeComponent<ecs::Scale>(entity);
            if (registry.hasComponent<ecs::Texture>(entity))
                registry.removeComponent<ecs::Texture>(entity);
            if (registry.hasComponent<ecs::LoadingAnimation>(entity))
                registry.removeComponent<ecs::LoadingAnimation>(entity);
        }
    }
} // namespace cli
