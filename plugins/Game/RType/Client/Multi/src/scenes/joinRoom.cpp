#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientMulti/Scenes/JoinRoom.hpp"
#include "Utils/Common.hpp"

namespace gme
{
    JoinRoomScene::JoinRoomScene(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                                 const std::shared_ptr<eng::IAudio> &audio)
        : AScene(assignedId), m_audio(audio)
    {
        auto &registry = AScene::getRegistry();

        registry.onComponentAdded(
            [&renderer, &audio, &registry](const ecs::Entity e, const std::type_info &type)
            {
                const auto *audioComp = registry.getComponent<ecs::Audio>(e);
                const auto *colorComp = registry.getComponent<ecs::Color>(e);
                const auto *fontComp = registry.getComponent<ecs::Font>(e);
                const auto *textComp = registry.getComponent<ecs::Text>(e);
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
                else if (type == typeid(ecs::Audio))
                {
                    if (audioComp)
                    {
                        audio->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                           audioComp->id + std::to_string(e));
                    }
                }
            });

        m_titleEntity =
            registry.createEntity()
                .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("transform_title", 100.F, 60.F, 0.F)
                .with<ecs::Color>("color_title", utl::Config::Color::CYAN_ELECTRIC.r, utl::Config::Color::CYAN_ELECTRIC.g, utl::Config::Color::CYAN_ELECTRIC.b, utl::Config::Color::CYAN_ELECTRIC.a)
                .with<ecs::Text>("title", std::string("JOIN ROOM"), 72U)
                .build();

        m_noRoomsEntity = registry.createEntity()
                              .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                              .with<ecs::Transform>("transform_no_rooms", 100.F, 200.F, 0.F)
                              .with<ecs::Color>("color_no_rooms", utl::Config::Color::INFO_TEXT_COLOR.r, utl::Config::Color::INFO_TEXT_COLOR.g,
                                                utl::Config::Color::INFO_TEXT_COLOR.b, utl::Config::Color::INFO_TEXT_COLOR.a)
                              .with<ecs::Text>("no_rooms_text", std::string("No rooms available"), 32U)
                              .build();

        m_refreshButtonEntity = registry.createEntity()
                                    .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                                    .with<ecs::Transform>("transform_refresh", 100.F, 400.F, 0.F)
                                    .with<ecs::Color>("color_refresh", utl::Config::Color::GRAY_BLUE_SUBTLE.r, utl::Config::Color::GRAY_BLUE_SUBTLE.g,
                                                      utl::Config::Color::GRAY_BLUE_SUBTLE.b, utl::Config::Color::GRAY_BLUE_SUBTLE.a)
                                    .with<ecs::Text>("refresh_text", std::string("Refresh"), 32U)
                                    .build();

        m_backButtonEntity = registry.createEntity()
                                 .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                                 .with<ecs::Transform>("transform_back", 100.F, 450.F, 0.F)
                                 .with<ecs::Color>("color_back", utl::Config::Color::GRAY_BLUE_SUBTLE.r, utl::Config::Color::GRAY_BLUE_SUBTLE.g,
                                                   utl::Config::Color::GRAY_BLUE_SUBTLE.b, utl::Config::Color::GRAY_BLUE_SUBTLE.a)
                                 .with<ecs::Text>("back_text", std::string("Back"), 32U)
                                 .build();
    }

    void JoinRoomScene::update(const float dt, const eng::WindowSize & /*size*/)
    {
        auto &reg = getRegistry();
        auto &colors = reg.getAll<ecs::Color>();
        auto &texts = reg.getAll<ecs::Text>();
        auto &audios = reg.getAll<ecs::Audio>();

        m_animationTime += dt;

        for (auto &audio : audios)
        {
            if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
                m_audio->stopAudio(audio.second.id);
        }

        const size_t totalOptions = m_rooms.size() + 2;

        for (auto &[entity, text] : texts)
        {
            if (text.id == "refresh_text")
            {
                auto &color = colors.at(entity);
                if (m_selectedIndex == m_rooms.size())
                {
                    float glowIntensity = std::sin(m_animationTime * 2.5f);
                    color.r = 0U;
                    color.g = static_cast<unsigned char>(191U + glowIntensity * 50);
                    color.b = 255U;
                }
                else
                {
                    color.r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
                    color.g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
                    color.b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
                }
            }
            else if (text.id == "back_text")
            {
                auto &color = colors.at(entity);
                if (m_selectedIndex == m_rooms.size() + 1)
                {
                    float glowIntensity = std::sin(m_animationTime * 2.5f);
                    color.r = 0U;
                    color.g = static_cast<unsigned char>(191U + glowIntensity * 50);
                    color.b = 255U;
                }
                else
                {
                    color.r = utl::Config::Color::GRAY_BLUE_SUBTLE.r;
                    color.g = utl::Config::Color::GRAY_BLUE_SUBTLE.g;
                    color.b = utl::Config::Color::GRAY_BLUE_SUBTLE.b;
                }
            }
        }

        for (size_t i = 0; i < m_roomEntities.size(); ++i)
        {
            if (auto *color = reg.getComponent<ecs::Color>(m_roomEntities[i]))
            {
                if (i == m_selectedIndex)
                {
                    float glowIntensity = std::sin(m_animationTime * 2.5f);
                    color->r = 0U;
                    color->g = static_cast<unsigned char>(191U + glowIntensity * 50);
                    color->b = 255U;
                }
                else
                {
                    color->r = utl::Config::Color::TEXT_VALUE_COLOR.r;
                    color->g = utl::Config::Color::TEXT_VALUE_COLOR.g;
                    color->b = utl::Config::Color::TEXT_VALUE_COLOR.b;
                }
            }
        }
    }

    void JoinRoomScene::event(const eng::Event &event)
    {
        const size_t totalOptions = m_rooms.size() + 2;

        switch (event.type)
        {
            case eng::EventType::KeyPressed:
                if (event.key == eng::Key::Escape)
                {
                    if (onBackToMulti)
                        onBackToMulti();
                }
                else if (event.key == eng::Key::Up)
                {
                    if (totalOptions > 0)
                        m_selectedIndex = (m_selectedIndex == 0) ? totalOptions - 1 : m_selectedIndex - 1;
                }
                else if (event.key == eng::Key::Down)
                {
                    if (totalOptions > 0)
                        m_selectedIndex = (m_selectedIndex == totalOptions - 1) ? 0 : m_selectedIndex + 1;
                }
                else if (event.key == eng::Key::Enter)
                {
                    if (m_selectedIndex < m_rooms.size() && onJoin)
                    {
                        onJoin(m_rooms[m_selectedIndex].roomId);
                    }
                    else if (m_selectedIndex == m_rooms.size() && onRefreshRequest)
                    {
                        onRefreshRequest();
                    }
                    else if (m_selectedIndex == m_rooms.size() + 1 && onBackToMulti)
                    {
                        onBackToMulti();
                    }
                }
                break;
            case eng::EventType::KeyReleased:
                break;
            default:
                break;
        }
    }

    void JoinRoomScene::setRooms(const std::vector<RoomInfo> &rooms)
    {
        m_rooms = rooms;
        updateRoomDisplay();
    }

    void JoinRoomScene::refreshRoomList()
    {
        if (onRefreshRequest)
            onRefreshRequest();
    }

    void JoinRoomScene::updateRoomDisplay()
    {
        auto &registry = getRegistry();

        clearRoomEntities();

        if (auto *noRoomsText = registry.getComponent<ecs::Text>(m_noRoomsEntity))
        {
            noRoomsText->content = m_rooms.empty() ? "No rooms available" : "";
        }

        for (size_t i = 0; i < m_rooms.size(); ++i)
        {
            const RoomInfo &room = m_rooms[i];
            std::string roomText =
                room.name + " " + std::to_string(room.currentPlayers) + "/" + std::to_string(room.maxPlayers);

            ecs::Entity roomEntity =
                registry.createEntity()
                    .with<ecs::Font>("main_font", utl::Path::Font::FONTS_RTYPE)
                    .with<ecs::Transform>("transform_room_" + std::to_string(i), 100.F, 200.F + i * 40.F, 0.F)
                    .with<ecs::Color>("color_room_" + std::to_string(i), utl::Config::Color::TEXT_VALUE_COLOR.r, utl::Config::Color::TEXT_VALUE_COLOR.g,
                                      utl::Config::Color::TEXT_VALUE_COLOR.b, utl::Config::Color::TEXT_VALUE_COLOR.a)
                    .with<ecs::Text>("room_" + std::to_string(i), roomText, 28U)
                    .build();

            m_roomEntities.push_back(roomEntity);
        }

        if (m_selectedIndex >= m_rooms.size() + 2)
        {
            m_selectedIndex = 0;
        }
    }

    void JoinRoomScene::clearRoomEntities()
    {
        auto &registry = getRegistry();

        for (ecs::Entity entity : m_roomEntities)
        {
            if (auto *text = registry.getComponent<ecs::Text>(entity))
                registry.removeComponent<ecs::Text>(entity);
            if (auto *color = registry.getComponent<ecs::Color>(entity))
                registry.removeComponent<ecs::Color>(entity);
            if (auto *transform = registry.getComponent<ecs::Transform>(entity))
                registry.removeComponent<ecs::Transform>(entity);
            if (auto *font = registry.getComponent<ecs::Font>(entity))
                registry.removeComponent<ecs::Font>(entity);
        }

        m_roomEntities.clear();
    }
} // namespace gme
