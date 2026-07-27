#include "application.h"
#include <fmt/printf.h>

int main(int argc, char* argv[]) {
    Application app;
    try {
        if (app.initialize()) {
            app.run();
        }
    }catch (const std::runtime_error& e) {
        fmt::print("ERROR: {}", e.what());
    }
    app.shutdown();

    return 0;
}
