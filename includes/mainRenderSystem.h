//
// Created by stefano on 28/07/26.
//

#pragma once

#include "renderSystem.h"

struct MainRenderContext : RenderContext {
    Image colorImage;
    Image depthImage;

    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;
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

    void init(VkDevice device, MainPipelineContext& pipelineCtx) override;

    void createPipeline(VkDevice device, MainPipelineContext& pipelineCtx) override;

    void destroyRenderSystem() override;
};
