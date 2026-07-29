//
// Created by stefano on 28/07/26.
//

#pragma once

#include "renderSystem.h"

class MainRenderSystem : RenderSystem {
public:
    void render(RenderContext &ctx) override;

    void init(VkDevice device, PipelineContext& pipelineCtx) override;

    void createPipeline(VkDevice device, PipelineContext& pipelineCtx) override;

    void createShader(VkDevice device) override;

    void destroyRenderSystem() override;
};
