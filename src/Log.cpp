#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace pal
{
    const char* Log::DEFAULT_FMT = "[%T %z] [%n] %l: %v%$";
    std::shared_ptr<spdlog::logger> Log::s_Logger;

    void Log::init(const char* loggerTitle, const spdlog::level::level_enum level, const char* filePath)
    {
        spdlog::set_pattern(DEFAULT_FMT);
        if (filePath)
            s_Logger = spdlog::basic_logger_mt(loggerTitle, filePath);
        else
            s_Logger = spdlog::stdout_color_mt(loggerTitle);
        s_Logger->set_level(level);
    }
}
