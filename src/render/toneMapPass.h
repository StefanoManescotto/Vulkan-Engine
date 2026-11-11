//
// Created by stefano on 28/07/26.
//

#pragma once

// #include "render/frameLayout.h"
// #include "render/renderer.h"
// #include "render/renderer.h"

// #include "scene/scene.h"
#include "vk/GPUImage.h"
#include "vk/device.h"
#include "vk/pipelineManager.h"
#include "vk/descriptorSets.h"
#include "vk/device.h"
// struct RenderItem {
// 	glm::mat4 wvp;
// 	glm::mat4 worldMatrix;
// 	uint32_t materialIndex = 0;
// };

// struct DrawItem {
//     uint32_t pipelineId;
//     uint32_t materialIndex;
//     uint32_t indexCount, firstIndex;
//     int32_t vertexOffset;
//     glm::mat4 wvp;
//     glm::mat4 worldMatrix;
// };

struct ToneMapContext {
    VkCommandBuffer cmd = nullptr;
    VkExtent2D extent = VkExtent2D(800, 600);

    // Scene* scene;

    GPUImage colorImage;
    // GPUImage depthImage;

    // VkShaderModule vertexShader;
    // VkShaderModule fragmentShader;

    // VkDrawIndexedIndirectCommand *indirectDrawPtr = nullptr;
	// RenderItem *renderItemPtr = nullptr;

    // GPUBufferVec* buffers;
    // GPUBuffer* renderItemBuffer;
    // GPUBuffer* indirectDrawBuffer;
};

// struct MainPipelineContext : PipelineContext {
//     VkShaderModule vertexShader;
//     VkShaderModule fragmentShader;

//     MainPipelineContext(VkFormat color, VkFormat depth, VkShaderModule vert, VkShaderModule frag) : PipelineContext{color, depth},
//     vertexShader(std::move(vert)),
//     fragmentShader(std::move(frag)) {}
// };

class ToneMapPass {
public:
    void init(const Device* device, ShaderManager& sManager, PipelineManager& pManager);
    void render(PipelineManager& pManager, ToneMapContext& ctx);

    void destroyRenderSystem();

private:
    Pipeline* tonePipeline = nullptr;
    DescriptorSets m_dsSet;
};
