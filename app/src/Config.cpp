#include "../include/Config.h"
#include "libconfig.h++"

bool loadConfig(const char* path, GameConfig& config)
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
        return false;
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

    LOG_INFO("Config loaded from {}", path);
    return true;
}

bool createConfigFile(const char* path, const GameConfig& config)
{
    const libconfig::Config cfg;

    using libconfig::Setting;
    Setting& root = cfg.getRoot();

    root.add("saveGamePath", Setting::TypeString) = config.saveGamePath.c_str();
    root.add("renderDistance", Setting::TypeInt) = (int32_t) config.renderDistance;
    root.add("loadDistance", Setting::TypeInt) = (int32_t) config.loadDistance;
    root.add("maxLoadsPerFrame", Setting::TypeInt) = (int32_t) config.maxLoadsPerFrame;
    root.add("maxUnloadsPerFrame", Setting::TypeInt) = (int32_t) config.maxUnloadsPerFrame;
    root.add("threadCount", Setting::TypeInt) = (int32_t) config.threadCount;
    root.add("maxBakesPerFrame", Setting::TypeInt) = (int32_t) config.maxBakesPerFrame;
    root.add("worldSeed", Setting::TypeInt) = (int32_t) config.worldSeed;
    root.add("reachDistance", Setting::TypeFloat) = config.reachDistance;

    try
    {
        cfg.writeFile(path);
        LOG_INFO("Created default config file at {}", path);
    }
    catch (const libconfig::FileIOException& e)
    {
        LOG_ERROR("I/O error while writing config file {}", path);
        return false;
    }

    return true;
}
