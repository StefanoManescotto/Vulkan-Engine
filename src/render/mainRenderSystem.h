//
// Created by stefano on 28/07/26.
//

#pragma once

#include "render/renderSystem.h"
// #include "render/renderer.h"
#include "scene/scene.h"
#include "scene/textureStorage.h"
#include "vk/GPUImage.h"
#include "vk/gpuBuffer.h"

struct RenderItem {
	glm::mat4 wvp;
	glm::mat4 worldMatrix;
	uint32_t materialIndex = 0;
};

struct FrameConstants {
	uint64_t vertexBufferAddress = 0;
	uint64_t materialBufferAddress = 0;
	uint64_t renderItemsAddress = 0;
};

struct MainRenderContext : RenderContext {
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
    void render2(MainRenderContext &ctx, TextureStorage &texStorage);

    void init(VkDevice device, MainPipelineContext& pipelineCtx) override;
    void init2(VkDevice device, MainPipelineContext& pipelineCtx, TextureStorage texStorage);

    void createPipeline(VkDevice device, MainPipelineContext& pipelineCtx) override;

    void destroyRenderSystem() override;
};
