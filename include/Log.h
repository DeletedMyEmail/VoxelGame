#pragma once

#include <spdlog/spdlog.h>

#ifndef NOLOG
    #define LOG_INIT_SPECIFIC(title, lvl, filepath) pal::Log::init(title, lvl, filepath)
    #define LOG_INIT() pal::Log::init(PROJECT_NAME)

    #define LOG_WARN(x) pal::Log::getLogger()->warn(x)
    #define LOG_INFO(x) pal::Log::getLogger()->info(x)
    #define LOG_ERROR(x) pal::Log::getLogger()->error(x)
#else
    #define LOG_INIT_SPECIFIC(title, lvl, filepath)
    #define LOG_INIT()

    #define LOG_WARN(x)
    #define LOG_INFO(x)
    #define LOG_ERROR(x)
#endif


namespace pal
{
    class Log
    {
    public:
        static void init(const char* loggerTitle = "App", spdlog::level::level_enum level = spdlog::level::trace, const char* filePath = nullptr);
        static std::shared_ptr<spdlog::logger>& getLogger() { return s_Logger; }
    public:
        static const char* DEFAULT_FMT;
    private:
        static std::shared_ptr<spdlog::logger> s_Logger;
    };

}
