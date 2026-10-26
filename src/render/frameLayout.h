#pragma once

#include <volk.h>

struct FrameLayout {
    // Set 0: global data — camera matrices, time, material buffer address...
    VkDescriptorSetLayout globalSetLayout;    // your TextureStorage already implies one!
    // Push constants: the shared stage layout
    VkPushConstantRange framePushConstant;   // FrameConstants
    VkPushConstantRange drawPushConstant;    // drawIndex / materialIndex
    VkPipelineLayout    defaultPipelineLayout; // set0 + push constants
};
