#include <ranges>

#include "R-Type/Renderer/EventHandler.hpp"
#include "Utils/Logger.hpp"

void rtp::EventHandler::publish(const sf::Event &e)
{
    for (auto &val : m_subscribers | std::views::values)
    {
        std::erase_if(val,
                      [&](const AnyHandler &h)
                      {
                          try
                          {
                              h(e);
                              return false;
                          }
                          catch (const std::exception &ex)
                          {
                              utl::Logger::log("[EventBus] Handler threw exception: " + std::string(ex.what()),
                                               utl::LogLevel::WARNING);
                              return true;
                          }
                          catch (...)
                          {
                              utl::Logger::log("[EventBus] Handler threw unknown exception", utl::LogLevel::WARNING);
                              return true;
                          }
                      });
    }
}