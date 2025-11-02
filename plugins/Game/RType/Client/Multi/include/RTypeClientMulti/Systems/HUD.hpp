///
/// @file HUD.hpp
/// @brief Heads-Up Display (HUD) system for R-Type multiplayer game
/// @details This system manages the on-screen UI elements including score display,
///          player statistics, and other game information overlays. It provides
///          real-time visual feedback to the player during gameplay.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "ECS/Interfaces/ISystems.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Common.hpp"

namespace gme
{

    ///
    /// @class HUDSystem
    /// @brief System responsible for rendering and updating the game's Heads-Up Display
    /// @details This ECS system manages all HUD elements including:
    ///          - Score display with digit sprites
    ///          - Background elements for UI components
    ///          - Real-time score updates
    ///
    /// The HUD is created at initialization and updated each frame to reflect
    /// the current game state. It uses sprite-based rendering for visual elements.
    ///
    /// @namespace gme
    ///
    class HUDSystem final : public ecs::ASystem
    {
        public:
            ///
            /// @brief Constructor - initializes the HUD system and creates initial UI elements
            /// @param renderer Shared pointer to the renderer for drawing UI elements
            /// @param registry Reference to the ECS registry for entity management
            /// @details Automatically creates the score HUD at position (10, 10) upon construction
            ///
            explicit HUDSystem(const std::shared_ptr<eng::IRenderer> &renderer, ecs::Registry &registry)
                : m_renderer(renderer), m_registry(registry)
            {
                createScoreHUD(m_registry, 10.0F, 10.0F);
            }

            ///
            /// @brief Destructor
            ///
            ~HUDSystem() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            HUDSystem(const HUDSystem &) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            HUDSystem &operator=(const HUDSystem &) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            HUDSystem(HUDSystem &&) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            HUDSystem &operator=(HUDSystem &&) = delete;

            ///
            /// @brief Update the HUD system (called each frame)
            /// @param registry ECS registry containing all entities and components
            /// @param dt Delta time (unused in current implementation)
            /// @details Updates score display and other HUD elements based on current game state
            ///
            void update(ecs::Registry &registry, float /* dt */) override;

        private:
            ///
            /// @brief Create the score display HUD elements
            /// @param registry ECS registry for creating entities
            /// @param x X-coordinate for the score display position
            /// @param y Y-coordinate for the score display position
            /// @details Creates background and digit entities for the score display.
            ///          Initializes the visual representation of the player's score.
            ///
            void createScoreHUD(ecs::Registry &registry, float x, float y);

            ///
            /// @brief Update the displayed score value
            /// @param registry ECS registry containing score entities
            /// @param newScore New score value to display
            /// @details Updates the sprite indices of digit entities to reflect the new score.
            ///          Handles score formatting and digit separation.
            ///
            void updateScore(ecs::Registry &registry, int newScore) const;

            const std::shared_ptr<eng::IRenderer> &m_renderer; ///< Renderer for drawing HUD elements
            ecs::Registry &m_registry;                         ///< ECS registry reference
            ecs::Entity m_scoreBgEntity{};                     ///< Entity for score background sprite
            std::vector<ecs::Entity> m_scoreDigitEntities;     ///< Entities for individual score digits
    }; // class HUDSystem

} // namespace gme
