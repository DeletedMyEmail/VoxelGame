#include "../include/Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Log::sLogger;

void Log::init() {
    spdlog::set_pattern("[%T %z] [thread %t] %n: %v%$");
    sLogger = spdlog::stdout_color_mt("Simulation");
    sLogger->set_level(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger>& Log::getLogger() {
    return sLogger;
}