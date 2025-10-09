#pragma once
#include <memory>

class SettingsManager {
public:
    void setVolume(int v) { m_volume = v; }
    int getVolume() const { return m_volume; }

    void setFullscreen(bool f) { m_fullscreen = f; }
    bool isFullscreen() const { return m_fullscreen; }

    void setFps(int fps) { m_fps = fps; }
    int getFps() const { return m_fps; }

private:
    int m_volume = 50;
    bool m_fullscreen = false;
    int m_fps = 240;
};
