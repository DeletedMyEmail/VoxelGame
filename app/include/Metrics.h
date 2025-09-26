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
    std::unordered_map<std::string, double> timer;
};

#ifdef NOPROFILE
    #define TIME(metrics, name, func) func;
#else
    #define PROFILE(metrics, name, func) { \
        double start = core::Application::get().getTime(); \
        func; \
        double end = core::Application::get().getTime(); \
        metrics.timer[name] = end - start; \
    }
#endif