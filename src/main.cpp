#include "application.h"

int main(int argc, char* argv[]) {
    Application app;
    if (app.initialize()) {
        app.run();
    }
    app.shutdown();

    return 0;

    // // Initialize SDL3 Video subsystem
    // if (!SDL_Init(SDL_INIT_VIDEO)) {
    //     std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    //     return -1;
    // }
    //
    // // Create window with Vulkan support
    // SDL_Window* window = SDL_CreateWindow(
    //     "Vulkan + SDL3 Test Window",
    //     800, 600,
    //     SDL_WINDOW_VULKAN
    // );
    //
    // if (!window) {
    //     std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
    //     SDL_Quit();
    //     return -1;
    // }
    //
    // // Query the extensions SDL3 requires to interface with Vulkan
    // uint32_t extensionCount = 0;
    // const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    //
    // std::cout << "Success! SDL3 initialized.\n";
    // std::cout << "Vulkan extensions required by SDL3 for your platform:\n";
    // for (uint32_t i = 0; i < extensionCount; ++i) {
    //     std::cout << " - " << extensions[i] << "\n";
    // }
    //
    // // Simple SDL3 event loop
    // bool running = true;
    // SDL_Event event;
    // while (running) {
    //     while (SDL_PollEvent(&event)) {
    //         if (event.type == SDL_EVENT_QUIT) {
    //             running = false;
    //         }
    //     }
    // }
    //
    // SDL_DestroyWindow(window);
    // SDL_Quit();
    return 0;
}