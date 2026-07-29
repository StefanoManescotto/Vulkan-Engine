#include "application.h"
// #include "utils.h"

#include <SDL3/SDL.h>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#define VMA_IMPLEMENTATION
#include <thirdparty/vk_mem_alloc.h>

#include <fmt/printf.h>
#include <fstream>

void Application::showError(const std::string &errorMessage) const {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errorMessage.c_str(), window.handle());
}

void Application::manageInputs() {
    if (window.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running = false;
    }
    if (window.isKeyPressed(SDL_SCANCODE_A)) {
        fmt::print("Pressed A - {} {}\n", window.getMousePosition().x, window.getMousePosition().y);
    }
    if (window.isKeyPressed(SDL_SCANCODE_S)) {
        fmt::print("Pressed S - {}\n", window.getMouseDelta());
    }

    window.updateInputState();
}

bool Application::initialize() {
    window.createWindow();

    if (!initializeVulkan()) {
        return false;
    }
    return true;
}

bool Application::initializeVulkan() {
    if (!createVulkanInstance()) {
        showError("Couldn't create a vulkan instance");
        return false;
    }

    window.createSurface(vulkanInstance);

    device.create(window.getSurface());
    device.findPhysicalDevice(vulkanInstance);
    device.findGraphicsQueue();
    device.createDevice();

    if (!initializeVMA()) {
        showError("Unable to create Vulkan Memory Allocator");
        return false;
    }

    renderer.init(&device, &vmaAllocator, &window);

    return true;
}

void Application::shutdown() {
    // wait in case resources are in use
    vkDeviceWaitIdle(device.handle());

    renderer.destroyRenderer();

    if (vmaAllocator) {
        vmaDestroyAllocator(vmaAllocator);
    }

    window.destroySurface();

    device.destroyDevice();

    if (vulkanInstance) {
        vkDestroyInstance(vulkanInstance, nullptr);
    }
    volkFinalize();

    window.destroyWindow();
}

void Application::run() {
    running = true;
    while (running) {
        if (!window.pollEvents()) {
            running = false;
            break;
        }
        manageInputs();

        render();
    }
}

bool Application::createVulkanInstance() {
    // Initialize Volk and load Vk function pointers
    if (volkInitialize() != VK_SUCCESS) {
        showError("Error initializing Volk");
        return false;
    }

    // Create the vulkan application instance
    VkApplicationInfo appInfo
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "My First Triangle",
        .apiVersion = VulkanVersion,
    };

    uint32_t instExtCount = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&instExtCount);

    std::vector requestedLayers {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = instExtCount,
        .ppEnabledExtensionNames = extensions
    };

    if (vkCreateInstance(&instCreateInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
        return false;
    }

    volkLoadInstance(vulkanInstance);
    return true;
}

bool Application::initializeVMA() {
    VmaVulkanFunctions vmaFuncInfo{};
    VmaAllocatorCreateInfo vmaAllocInfo {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = device.getPhysicalDevice(),
        .device = device.handle(),
        .pVulkanFunctions = &vmaFuncInfo,
        .instance = vulkanInstance,
        .vulkanApiVersion = VulkanVersion
    };

    // vma can import directly from volk
    vmaImportVulkanFunctionsFromVolk(&vmaAllocInfo, &vmaFuncInfo);

    if (vmaCreateAllocator(&vmaAllocInfo, &vmaAllocator) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void Application::render() {
    renderer.renderFrame(&window);
}
