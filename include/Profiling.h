#pragma once

#include <cstdint>
#include <string>
#include <functional>

#ifndef NOPROFILE
    #define PROFILER_INIT(maxStamps) pal::Profiler::init(maxStamps)
    #define PROFILER_END() pal::Profiler::end()

    #define PROFILE_SCOPE_NAMED(name) pal::Timer timer(name)
    #define PROFILE_SCOPE() PROFILE_SCOPE_NAMED(__FUNCTION__)

    #define REP_TEST(func, bytesProcessed, maxTotalReps, maxRepsSinceLastMin) pal::repetitionTest(func, bytesProcessed, maxTotalReps, maxRepsSinceLastMin)
#else
    #define PROFILER_INIT(maxStamps)
    #define PROFILER_END()

    #define PROFILE_SCOPE_NAMED(name)
    #define PROFILE_SCOPE()

    #define REP_TEST(func, bytesProcessed, maxTotalReps, maxRepsSinceLastMin) pal::RepetitionResult{}
#endif

namespace pal
{
    struct TimeStamp
    {
        uint64_t tscs;
        const char* name;
    };

    class Profiler{
    public:
        static void init(uint32_t maxTimeStamps = 10);
        static void end();
        static void addTimeStamp(TimeStamp timeStamp);

        static uint32_t getMaxTimeStamps() { return s_MaxStampCount; }
        static uint32_t getTimeStampCount() { return s_StampCount; }
    private:
        static uint64_t s_StartTime;
        static TimeStamp* s_TimeStamps;
        static uint32_t s_StampCount, s_MaxStampCount;
    };

    class Timer
    {
    public:
        Timer(const char* name);
        ~Timer();
    private:
        const char* m_Name;
        uint64_t m_StartTime;
    };

    double tscsToMs(uint64_t tscs);

    struct RepetitionResult
    {
        uint64_t minTSCs, maxTSCs;
        // bytes per second
        double maxThroughput, minThroughput;
        uint32_t repetitions, bestRepIndex, worstRepIndex;

        operator std::string() const;
    };

    RepetitionResult repetitionTest(const std::function<void()>& func, uint64_t bytesProcessed = 0, uint32_t maxTotalReps = 512, uint32_t maxRepsSinceLastMin = 16);
}