//
// Created by stefano on 28/07/26.
//

#pragma once

#include "pipeline.h"
#include "shader.h"
#include "image.h"

struct RenderContext {
    VkCommandBuffer cmd = nullptr;
    VkExtent2D extent = VkExtent2D(800, 600);

    // VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    // VkFormat depthFormat = VK_FORMAT_UNDEFINED;

    // VkImage colorImg = nullptr;
    // VkImage depthImg = nullptr;
    //
    // VkImageView color = nullptr;
    // VkImageView depth = nullptr;
    // VkImageView normal = nullptr;
    // VkImageView motion = nullptr;
};

struct PipelineContext {
    VkFormat colorFormat = VK_FORMAT_UNDEFINED;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    // VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

template <typename StructType>
class RenderSystem {
public:
    virtual ~RenderSystem() = default;

    virtual void init(VkDevice device, PipelineContext& pipelineCtx)= 0;
    virtual void render(StructType& ctx)= 0;

    virtual void createPipeline(VkDevice device, PipelineContext& pipelineCtx)= 0;
    virtual void createShader(VkDevice device)= 0;

    virtual void destroyRenderSystem()= 0;

protected:
    Shader shader;
    Pipeline pipeline;
    PipelineConfig pipelineConfig;
};
