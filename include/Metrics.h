#pragma once
#include <algorithm>
#include <chrono>
#include <deque>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <GLFW/glfw3.h>

struct Metrics
{
    Metrics() : lastTime(glfwGetTime()) {}
    void update();
    float getAvgFrameTime() const;
    float get1PercentLowFrameTime() const;

    float lastTime = 0, frameTimeAccumulator = 0, frameTimeWindow = 5.0f, deltaTime = 0;
    std::deque<float> frameTimes;
    std::unordered_map<std::string, int64_t> timer;
};

inline int64_t currenTime()
{
    using namespace std::chrono;
    return duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

#ifdef NOPROFILE
    #define TIME(metrics, name, func) func;
#else
    #define TIME(metrics, name, func) { \
        int64_t start = currenTime(); \
        func; \
        int64_t end = currenTime(); \
        metrics.timer[name] = end - start; \
    }
#endif