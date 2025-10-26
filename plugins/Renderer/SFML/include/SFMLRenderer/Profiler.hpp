///
/// @file Profiler.hpp
/// @brief This file contains the Profiler class declaration
/// @namespace eng
///

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace eng
{
    class Profiler
    {
        public:
            using Clock = std::chrono::high_resolution_clock;
            using TimePoint = Clock::time_point;

            struct Stat
            {
                std::string name;
                double lastMs = 0.0;
                std::vector<double> history;
            };

            void start(const std::string& key) { m_startTimes[key] = Clock::now(); }

            void end(const std::string& key) {
                const auto end = Clock::now();
                double ms = std::chrono::duration<double, std::milli>(end - m_startTimes[key]).count();
                auto &[name, lastMs, history] = m_stats[key];
                name = key;
                lastMs = ms;
                history.push_back(ms);
                if (history.size() > 100) { history.erase(history.begin());
                }
            }

            [[nodiscard]] const std::unordered_map<std::string, Stat>& getStats() const { return m_stats; }

        private:
            std::unordered_map<std::string, TimePoint> m_startTimes;
            std::unordered_map<std::string, Stat> m_stats;
    };

}