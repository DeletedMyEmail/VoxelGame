#include "App.h"
#include "Log.h"

int main() {
    Log::init();
    App app{};
    app.run();

    return 0;
}