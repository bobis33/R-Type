#include <algorithm>

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include "SFMLRenderer/Profiler.hpp"
#include "SFMLRenderer/SFMLRenderer.hpp"

void eng::SFMLRenderer::updateFrame(Profiler &profiler, const WindowSize &windowSize)
{
    profiler.start("frame");

    ImGui::SFML::Update(window, deltaClock.restart());
    ImGui::SetNextWindowPos(ImVec2(windowSize.width - 10.0F, 10.0F), ImGuiCond_Always, ImVec2(1.0F, 0.0F));
    ImGui::SetNextWindowBgAlpha(0.35F);
    constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                              ImGuiWindowFlags_NoNav;
    ImGui::Begin("Profiling", nullptr, window_flags);

    const float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f (%.3f ms/frame)", fps, 1000.0F / fps);

    for (const auto &[name, stat] : profiler.getStats())
        {
        ImGui::Text("%s: %.3f ms", name.c_str(), stat.lastMs);
        if (!stat.history.empty()) {
            std::vector<float> history_f(stat.history.begin(), stat.history.end());
            ImGui::PlotLines(("##" + name).c_str(), history_f.data(), static_cast<int>(history_f.size()), 0, nullptr,
                             0.0F, *std::ranges::max_element(history_f), ImVec2(0, 50));
        }
    }

    ImGui::End();
    ImGui::SFML::Render(window);
    profiler.end("frame");
}

void eng::SFMLRenderer::renderGui(const WindowSize &windowSize)
{
    ImGui::SFML::Update(window, deltaClock.restart());
    ImGui::SetNextWindowPos(ImVec2(windowSize.width - 10.0F, 10.0F), ImGuiCond_Always, ImVec2(1.0F, 0.0F));
    ImGui::SetNextWindowBgAlpha(0.35F);

    constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                              ImGuiWindowFlags_NoNav;
    ImGui::Begin("FPS Overlay", nullptr, window_flags);
    const float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f (%.3f ms/frame)", fps, 1000.0F / fps);
    ImGui::End();
    // ImGui::Text("Active entities: %zu", m_engine ? m_engine->getSceneManager()->getCurrentScene().getRegistry().size() : 0);

    updateFrame(m_profiler, windowSize);
    ImGui::SFML::Render(window);
}