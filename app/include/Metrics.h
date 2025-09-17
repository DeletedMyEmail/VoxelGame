#pragma once
#include <chrono>
#include <deque>
#include <string>
#include <unordered_map>

struct Metrics
{
    void update(double dt);
    double getAvgFrameTime() const;
    double get1PercentLowFrameTime() const;

    double frameTimeAccumulator = 0;
    double frameTimeWindow = 5.0f;

    std::deque<double> frameTimes;
    std::unordered_map<std::string, int64_t> timer;
};

#ifdef NOPROFILE
    #define TIME(metrics, name, func) func;
#else
    #define TIME(metrics, name, func) { \
        int64_t start = core::Application::get().getTime(); \
        func; \
        int64_t end = core::Application::get().getTime(); \
        metrics.timer[name] = end - start; \
    }
#endif