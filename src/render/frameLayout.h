#pragma once

#include <volk.h>

struct FrameLayout {
    VkDescriptorSetLayout globalSetLayout;
    VkPushConstantRange framePushConstant;
    VkPushConstantRange drawPushConstant;
    VkPipelineLayout    defaultPipelineLayout;
};
