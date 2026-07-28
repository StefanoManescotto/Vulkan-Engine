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
#include "shader.h"
#include "window.h"
#include "pipeline.h"

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
public:
    bool initialize();
    void shutdown();
    void run();

private:
    constexpr static uint32_t VulkanVersion{VK_API_VERSION_1_4};
    constexpr static uint32_t MaxFramesInFlight{2};

    Window window;
    uint32_t width = 1280;
    uint32_t height = 720;
    bool running = false;
    uint64_t frameIndex = 0;
    uint64_t nextSignalValue = MaxFramesInFlight + 1;

    // vulkan core
    VkInstance vulkanInstance = nullptr;
    Device device;
    VmaAllocator vmaAllocator = nullptr;

    // swapchain related
    Swapchain swapchain;
    SwapchainConfig config;
    bool requireSwapchainRecreate = false;

    // graphics pipeline related
    Pipeline pipeline;
    PipelineConfig pipelineConfig;
    // VkPipelineLayout pipelineLayout = nullptr;
    // VkPipeline pipeline = nullptr;

    // shader resources
    Shader shader;

    // frame and synchronization resources
    VkSemaphore timelineSemaphore = nullptr;
    std::array<FrameResources, MaxFramesInFlight> frameResources;

    void showError(const std::string &errorMessasge) const;

    bool initializeVulkan();
    bool createVulkanInstance();

    // bool createSurface();

    bool findGraphicsQueue();

    bool initializeVMA();

    VkPipeline createGraphicsPipeline();
    bool createSyncResources();
    bool createCommandBuffers();

    void render();

    void manageInputs();
};
