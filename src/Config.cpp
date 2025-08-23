#include "Config.h"
#include "libconfig.h++"

bool loadConfig(const char* path, ProgramConfig& config)
{
    libconfig::Config cfg;

    try
    {
        cfg.readFile(path);
    }
    catch(const libconfig::FileIOException& e)
    {
        LOG_ERROR("I/O error while reading config file {}", path);
        return false;
    }
    catch(const libconfig::ParseException& e)
    {
        LOG_ERROR("Parse error in config file {} at line {}: {}", e.getFile(), e.getLine(), e.getError());
        return false;
    }

    try
    {
        if (cfg.exists("saveGamePath"))
            config.saveGamePath = (const char*) cfg.lookup("saveGamePath");
        if (cfg.exists("renderDistance"))
            config.renderDistance = cfg.lookup("renderDistance");
        if (cfg.exists("loadDistance"))
            config.loadDistance = cfg.lookup("loadDistance");
        if (cfg.exists("threadCount"))
            config.threadCount = cfg.lookup("threadCount");
        if (cfg.exists("maxLoadsPerFrame"))
            config.maxLoadsPerFrame = cfg.lookup("maxLoadsPerFrame");
        if (cfg.exists("maxUnloadsPerFrame"))
            config.maxUnloadsPerFrame = cfg.lookup("maxUnloadsPerFrame");
        if (cfg.exists("maxBakesPerFrame"))
            config.maxBakesPerFrame = cfg.lookup("maxBakesPerFrame");
        if (cfg.exists("worldSeed"))
            config.worldSeed = cfg.lookup("worldSeed");
        if (cfg.exists("reachDistance"))
            config.reachDistance = cfg.lookup("reachDistance");
    }
    catch (libconfig::SettingTypeException& e)
    {
        LOG_ERROR("Config error: invalid type - {}", e.what());
    }
    catch (libconfig::SettingRangeException& e)
    {
        LOG_ERROR("Config error: invalid number - {}", e.what());
        return false;
    }

    if (config.loadDistance < config.renderDistance)
    {
        LOG_WARN("Config warning: load distance is less than the render distance. It's capped to {}.", config.renderDistance);
        config.loadDistance = config.renderDistance;
    }

    if (config.threadCount > std::thread::hardware_concurrency())
        LOG_WARN("Config warning: threadCount is less than the number of CPU cores. It's capped to {}.", std::thread::hardware_concurrency());

    return true;
}