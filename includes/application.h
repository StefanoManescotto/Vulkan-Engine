#pragma once

#define VK_NO_PROTOTYPES
#include <SDL3/SDL_vulkan.h>
#include <string>
// #include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <shaderc/shaderc.hpp>

#include "swapchain.h"
#include "device.h"

struct SDL_Window;
struct VmaAllocator_T;
typedef VmaAllocator_T* VmaAllocator;
struct VmaAllocation_T;
typedef VmaAllocation_T* VmaAllocation;

struct FrameResources {
    VkCommandPool commandPool = nullptr;
    VkCommandBuffer commandBuffer = nullptr;

    VkSemaphore imageAcquiredSemaphore = nullptr;
};

class Application {
    constexpr static uint32_t VulkanVersion{VK_API_VERSION_1_4};
    constexpr static uint32_t MaxFramesInFlight{2};

    SDL_Window *window = nullptr;
    uint32_t width = 1280;
    uint32_t height = 720;
    bool running = false;
    uint64_t frameIndex = 0;
    uint64_t nextSignalValue = MaxFramesInFlight + 1;

    // vulkan core
    VkInstance vulkanInstance = nullptr;
    Device device;
    // VkPhysicalDevice physicalDevice = nullptr;
    // VkDevice device = nullptr;
    VkSurfaceKHR surface = nullptr;
    VmaAllocator vmaAllocator = nullptr;

    // queue related
    // uint32_t gfxQueueFamIdx = UINT32_MAX;
    // VkQueue gfxQueue = nullptr;

    // swapchain related
    Swapchain swapchain;
    SwapchainConfig config;
    bool requireSwapchainRecreate = false;

    // graphics pipeline related
    VkPipelineLayout pipelineLayout = nullptr;
    VkPipeline pipeline = nullptr;

    // shader resources
    VkShaderModule vertShader = nullptr;
    VkShaderModule fragShader = nullptr;

    // frame and synchronization resources
    VkSemaphore timelineSemaphore = nullptr;
    std::array<FrameResources, MaxFramesInFlight> frameResources;

    void showError(const std::string &errorMessasge) const;

    bool initializeVulkan();
    bool createVulkanInstance();

    bool createSurface();

    VkPhysicalDevice findPhysicalDevice();
    bool findGraphicsQueue();
    bool createDevice(VkPhysicalDevice physicalDevice);

    bool initializeVMA();

    VkShaderModule createShaderModule(const std::string &fileName, shaderc_shader_kind kind) const;
    bool createShaders();

    VkPipeline createGraphicsPipeline();
    bool createSyncResources();
    bool createCommandBuffers();

    void render();

public:
    bool initialize();
    void shutdown();
    void run();
};
