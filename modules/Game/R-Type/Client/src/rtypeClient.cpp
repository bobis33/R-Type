#include "R-TypeClient/LobbyScene.hpp"

#include "R-TypeClient/RTypeClient.hpp"

gme::RTypeClient::RTypeClient() : m_currentScene(std::make_unique<LobbyScene>())
{
    AGameClient::setName("R-Type");
    auto& entities = m_currentScene->getEntitiesMutable();
    entities.push_back(Sprite{
        .type="player",
        .pos_x=200.0F,
        .pos_y=100.0F,
        .v_x=0.0F, .v_y=0.0F,
        .texture_path ="assets/sprites/r-typesheet42.gif",
        .text_rect_x=0, .text_rect_y = 0, .text_rect_fx = 33, .text_rect_fy = 20,
        .id="player_0"
    });
    for (int i = 0; i < 100; i++) {
        entities.push_back(Sprite{
            .type="star",
            .pos_x=0, .pos_y=0,
            .v_x = -20.0F - static_cast<float>(std::rand() % 30),
            .r=255, .g=255, .b=255, .a=255,
            .id="star_" + std::to_string(i)
        });
    }
}

void gme::RTypeClient::update(const float deltaTime, const int width, const int height)
{
    for (auto& entities = m_currentScene->getEntitiesMutable(); auto& e : entities) {
        e.pos_x += e.v_x * deltaTime;
        e.pos_y += e.v_y * deltaTime;

        if (e.type == "star" && (e.pos_x < 2.0F || e.pos_y < 2.0F)) {
            e.pos_x = static_cast<float>(std::rand() % width);
            e.pos_y = static_cast<float>(std::rand() % height);
        }
    }
}