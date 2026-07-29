#pragma once

#define VK_NO_PROTOTYPES
#include <SDL3/SDL_vulkan.h>
#include <shaderc/shaderc.hpp>

#include "swapchain.h"
#include "device.h"
#include "window.h"

#include <renderer.h>

struct SDL_Window;
struct VmaAllocator_T;
typedef VmaAllocator_T* VmaAllocator;
struct VmaAllocation_T;
typedef VmaAllocation_T* VmaAllocation;

class Application {
public:
    void initialize();
    void shutdown();
    void run();

private:
    constexpr static uint32_t VulkanVersion{VK_API_VERSION_1_4};
    constexpr static uint32_t MaxFramesInFlight{2};

    Window window;
    uint32_t width = 1280;
    uint32_t height = 720;
    bool running = false;

    VkInstance vulkanInstance = nullptr;
    Device device;
    VmaAllocator vmaAllocator = nullptr;
    Renderer renderer;

    void initializeVulkan();
    void createVulkanInstance();

    void initializeVMA();

    void render();

    void manageInputs();
};
