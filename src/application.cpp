#include "application.h"

#include <SDL3/SDL.h>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#define VMA_IMPLEMENTATION
#include <thirdparty/vk_mem_alloc.h>

#include <fmt/printf.h>

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

void Application::initialize() {
    window.createWindow();

    initializeVulkan();
}

void Application::initializeVulkan() {
    createVulkanInstance();

    window.createSurface(vulkanInstance);

    device.create(window.getSurface());
    device.findPhysicalDevice(vulkanInstance);
    device.findGraphicsQueue();
    device.createDevice();

    initializeVMA();

    renderer.init(&device, &vmaAllocator, &window);
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

void Application::createVulkanInstance() {
    // Initialize Volk and load Vk function pointers
    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("Error initializing Volk");
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
        throw std::runtime_error("Couldn't create a vulkan instance");
    }

    volkLoadInstance(vulkanInstance);
}

void Application::initializeVMA() {
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
        throw std::runtime_error("Unable to create Vulkan Memory Allocator");
    }
}

void Application::render() {
    renderer.renderFrame(&window);
}
