//
// Created by stefano on 28/07/26.
//
#pragma once

#include <volk.h>
#include <vulkan/vulkan_core.h>

struct Pipeline {
    VkPipeline handle;
    VkPipelineLayout pipelineLayout;
};
