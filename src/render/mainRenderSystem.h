//
// Created by stefano on 28/07/26.
//

#pragma once

// #include "render/frameLayout.h"
// #include "render/renderer.h"
#include "render/renderSystem.h"
#include "scene/imGuiManager.h"
#include "scene/scene.h"
#include "scene/textureStorage.h"
#include "vk/GPUImage.h"
#include "vk/gpuBuffer.h"
#include "vk/pipelineManager.h"

struct RenderItem {
	glm::mat4 wvp;
	glm::mat4 worldMatrix;
	uint32_t materialIndex = 0;
};

struct DrawItem {
    uint32_t pipelineId;
    uint32_t materialIndex;
    uint32_t indexCount, firstIndex;
    int32_t vertexOffset;
    glm::mat4 wvp;
    glm::mat4 worldMatrix;
};

struct MainRenderContext {
    VkCommandBuffer cmd = nullptr;
    VkExtent2D extent = VkExtent2D(800, 600);

    Scene* scene;

    GPUImage colorImage;
    GPUImage depthImage;

    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;

    VkDrawIndexedIndirectCommand *indirectDrawPtr = nullptr;
	RenderItem *renderItemPtr = nullptr;

    GPUBufferVec* buffers;
    GPUBuffer* renderItemBuffer;
    GPUBuffer* indirectDrawBuffer;
};

struct MainPipelineContext : PipelineContext {
    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;

    MainPipelineContext(VkFormat color, VkFormat depth, VkShaderModule vert, VkShaderModule frag) : PipelineContext{color, depth},
    vertexShader(std::move(vert)),
    fragmentShader(std::move(frag)) {}
};

class MainRenderSystem : RenderSystem<MainRenderContext, MainPipelineContext> {
public:
    void render(MainRenderContext &ctx) override;
    void render2(MainRenderContext &ctx, TextureStorage &texStorage, PipelineManager& pManager, ImGuiManager& guiManager);

    void destroyRenderSystem() override;
};
