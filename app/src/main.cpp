#include "Application.h"
#include "ControlLayer.h"
#include "DebugLayer.h"
#include "GameLayer.h"
#include "cstmlib/Log.h"
#include "cstmlib/Profiling.h"

int main(int argc, char* argv[])
{
#ifdef RELEASE_BUILD
    LOG_INIT_SPECIFIC(PROJECT_NAME, spdlog::level::warn, "log.txt");
#else
    LOG_INIT();
#endif
    PROFILER_INIT(100);

    const auto CONFIG_PATH = "voxel.config";
    GameConfig gameConfig;
    if (std::filesystem::exists(CONFIG_PATH))
    {
        if (!loadConfig(CONFIG_PATH, gameConfig))
            return 1;
    }
    else if (!createConfigFile(CONFIG_PATH, gameConfig))
        return 1;

    WindowSettings windowSettings;
    windowSettings.title = PROJECT_NAME;
    windowSettings.fullscreen = true;
    core::Application app(windowSettings);
    app.pushLayer<core::Application::TOP, GameLayer>("GameLayer", gameConfig);
    app.pushLayer<core::Application::TOP, DebugLayer>("DebugLayer");
    app.pushLayer<core::Application::TOP, ControlLayer>("ControlLayer");
    app.run();

    PROFILER_END();
    return 0;
}
