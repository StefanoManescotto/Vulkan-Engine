//
// Created by stefano on 28/07/26.
//

#pragma once

#include <array>

#include "render/frameLayout.h"
#include "scene/imGuiManager.h"
#include "scene/nodeWorld.h"
#include "scene/textureStorage.h"
#include "vk/commandContext.h"
#include "vk/device.h"
#include "render/mainRenderSystem.h"
#include "shaderManager.h"
#include "vk/gpuBuffer.h"
#include "vk/swapchain.h"

#include "volk.h"

struct Window;

struct FrameConstants {
	uint64_t vertexBufferAddress = 0;
	uint64_t materialBufferAddress = 0;
	uint64_t renderItemsAddress = 0;
};

struct FrameResources {
    VkCommandPool commandPool = nullptr;
    VkCommandBuffer commandBuffer = nullptr;

    VkSemaphore imageAcquiredSemaphore = nullptr;
    uint32_t imageIndex = std::numeric_limits<uint32_t>::max();
    uint64_t signalValue = 0;

    GPUImage colorImage;
    GPUImage depthImage;

    GPUBuffer indirectDrawBuffer;
	GPUBuffer renderItemBuffer;
	VkDrawIndexedIndirectCommand *indirectDrawPtr = nullptr;
	RenderItem *renderItemPtr = nullptr;
};

class Renderer {
public:
    Renderer() = default;

    void init(const Device* device, const VmaAllocator* allocator, const Window* window, size_t maxNodes);
    // const PipelineManager& getPipelineManager() const { return m_pipelineManager; }

    void renderFrame(const Window* window, Scene &scene, GPUBufferVec &buffers, ImGuiManager& guiManager);
    
    // TODO: make these const
    CommandContext& getCommandContext() { return cmdContext; }
    TextureStorage& getTextureStorage() { return m_texStorage; }
    PipelineManager& getPipelineManager() { return m_pipelineManager; }
    ShaderManager& getShaderManager() { return m_shaderManager; }
    FrameLayout& getFrameLayout() { return m_frameLayout; }
    Swapchain& getSwapchain() { return swapchain; }
    void recreateSwapchain();

    void destroyRenderer();
private:
    constexpr static uint32_t MaxFramesInFlight { 2 };
    uint64_t nextSignalValue = MaxFramesInFlight + 1;
    uint64_t frameIndex = 0;
    MainRenderContext ctx;

    const Device* m_device;
    const VmaAllocator* m_allocator;

    std::vector<std::pair<Node *, glm::mat4>> m_nodeRenderStack;
    MainRenderSystem m_mainRenderSystem;
    std::array<FrameResources, MaxFramesInFlight> m_frameResources;
    FrameResources currentFrameRes;
    VkSemaphore timelineSemaphore = nullptr;
    CommandContext cmdContext;
    TextureStorage m_texStorage;
    FrameLayout m_frameLayout;

    Swapchain swapchain;
    SwapchainConfig config;
    bool requireSwapchainRecreate = false;

    ShaderManager m_shaderManager;
    PipelineManager m_pipelineManager;
    
    bool createIndirectDrawBuffers(size_t maxNodes);
    void createCommandBuffer();
    void createSyncResources();
    void destroySyncResourcesImages();

    /**
     * @brief Executes operations needed for rendering
     * @param window Needed for the surface.
     * @return True if rendering can proceed, False if the swapchain needs to be recreated.
     */
    bool beginFrame(const Window* window);
    void endFrame();

    void initImages();
};
