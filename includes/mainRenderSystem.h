//
// Created by stefano on 28/07/26.
//

#pragma once

#include "renderSystem.h"

struct MainRenderContext : RenderContext {
    Image colorImage;
    Image depthImage;
};

class MainRenderSystem : RenderSystem<MainRenderContext> {
public:
    void render(MainRenderContext &ctx) override;

    void init(VkDevice device, PipelineContext& pipelineCtx) override;

    void createPipeline(VkDevice device, PipelineContext& pipelineCtx) override;

    void createShader(VkDevice device) override;

    void destroyRenderSystem() override;
};
