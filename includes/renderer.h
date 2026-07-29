//
// Created by stefano on 28/07/26.
//

#pragma once

#include <array>

#include "device.h"
#include "mainRenderSystem.h"
// #include "renderSystem.h"
#include "swapchain.h"
#include "volk.h"

struct FrameResources {
    VkCommandPool commandPool = nullptr;
    VkCommandBuffer commandBuffer = nullptr;

    VkSemaphore imageAcquiredSemaphore = nullptr;
    uint32_t imageIndex = std::numeric_limits<uint32_t>::max();
    uint64_t signalValue = 0;
};

struct Window;

class Renderer {
public:
    Renderer() = default;

    void init(const Device* device, const VmaAllocator* allocator, const Window* window);

    bool beginFrame(const Window* window);
    void endFrame();
    void renderFrame(const Window* window);

    void destroyRenderer();
private:
    constexpr static uint32_t MaxFramesInFlight { 2 };
    uint64_t nextSignalValue = MaxFramesInFlight + 1;
    uint64_t frameIndex = 0;
    RenderContext ctx;

    const Device* m_device;

    MainRenderSystem m_mainRenderSystem;
    std::array<FrameResources, MaxFramesInFlight> m_frameResources;
    FrameResources currentFrameRes;
    VkSemaphore timelineSemaphore = nullptr;

    Swapchain swapchain;
    SwapchainConfig config;
    bool requireSwapchainRecreate = false;

    void createCommandBuffer();
    void createSyncResources();
};
