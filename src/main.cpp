#include "Profiling.h"
#include "Log.h"

void run();

int main() {
    LOG_INIT();
    PROFILER_INIT(10);

    run();

    PROFILER_END();
    return 0;
}