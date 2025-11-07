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

    if (argc >= 2 && !loadConfig(argv[1], gameConfig))
        return 1;

    WindowSettings windowSettings;
    windowSettings.title = PROJECT_NAME;
    windowSettings.fullscreen = true;
    core::Application app(windowSettings);
    app.pushLayer<core::Application::TOP, GameLayer>("GameLayer");
    app.pushLayer<core::Application::TOP, DebugLayer>("DebugLayer");
    app.pushLayer<core::Application::TOP, ControlLayer>("ControlLayer");
    app.run();

    PROFILER_END();
    return 0;
}
