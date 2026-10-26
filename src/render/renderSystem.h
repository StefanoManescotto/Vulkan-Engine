//
// Created by stefano on 28/07/26.
//

#pragma once

#include "vk/pipeline.h"

#include "vk/pipelineManager.h"

// struct RenderContext {
//     // TextureManager texManager;
//     VkCommandBuffer cmd = nullptr;
//     VkExtent2D extent = VkExtent2D(800, 600);
//     // RenderWorld objs;
// };

struct PipelineContext {
    VkFormat colorFormat = VK_FORMAT_UNDEFINED;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    // VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

template <typename RenderContextT, typename PipelineContextT>
class RenderSystem {
public:
    virtual ~RenderSystem() = default;

    virtual void init(VkDevice device, PipelineContextT& pipelineCtx)= 0;
    virtual void render(RenderContextT& ctx)= 0;

    virtual void createPipeline(VkDevice device, PipelineContextT& pipelineCtx)= 0;

    virtual void destroyRenderSystem()= 0;

protected:
    // Shader shader;
    Pipeline pipeline;
    PipelineConfig pipelineConfig;
    RenderContextT config;
};
