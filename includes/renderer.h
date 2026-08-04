//
// Created by stefano on 28/07/26.
//

#pragma once

#include <array>

#include "device.h"
#include "mainRenderSystem.h"
#include "shaderManager.h"
#include "swapchain.h"
#include "volk.h"

class GameObject;

struct FrameResources {
    VkCommandPool commandPool = nullptr;
    VkCommandBuffer commandBuffer = nullptr;

    VkSemaphore imageAcquiredSemaphore = nullptr;
    uint32_t imageIndex = std::numeric_limits<uint32_t>::max();
    uint64_t signalValue = 0;

    Image colorImage;
    Image depthImage;
};

struct Window;
struct Scene;

class Renderer {
public:
    Renderer() = default;

    void init(const Device* device, const VmaAllocator* allocator, const Window* window);

    void renderFrame(const Window* window, Scene scene);

    void destroyRenderer();
private:
    constexpr static uint32_t MaxFramesInFlight { 2 };
    uint64_t nextSignalValue = MaxFramesInFlight + 1;
    uint64_t frameIndex = 0;
    MainRenderContext ctx;

    const Device* m_device;
    const VmaAllocator* m_allocator;

    MainRenderSystem m_mainRenderSystem;
    std::array<FrameResources, MaxFramesInFlight> m_frameResources;
    FrameResources currentFrameRes;
    VkSemaphore timelineSemaphore = nullptr;

    Swapchain swapchain;
    SwapchainConfig config;
    bool requireSwapchainRecreate = false;

    ShaderManager m_shaderManager;

    void createCommandBuffer();
    void createSyncResources();

    /**
     * @brief Executes operations needed for rendering
     * @param window Needed for the surface.
     * @return True if rendering can proceed, False if the swapchain needs to be recreated.
     */
    bool beginFrame(const Window* window);
    void endFrame();

    void initImages();
};
