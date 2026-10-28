//
// Created by stefano on 28/07/26.
//

#pragma once

#include "vk/pipeline.h"
#include "vk/pipelineManager.h"

struct PipelineContext {
    VkFormat colorFormat = VK_FORMAT_UNDEFINED;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
};

template <typename RenderContextT, typename PipelineContextT>
class RenderSystem {
public:
    virtual ~RenderSystem() = default;
    virtual void render(RenderContextT& ctx)= 0;
    virtual void destroyRenderSystem()= 0;

protected:
    Pipeline pipeline;
    PipelineConfig pipelineConfig;
    RenderContextT config;
};
