#include "application.h"
#include <fmt/printf.h>

int main(int argc, char* argv[]) {
    try {
        Application app;
        app.initialize();
        app.run();
        app.shutdown();
    }catch (const std::runtime_error& e) {
        fmt::print("ERROR: {}", e.what());
    }

    return 0;
}
