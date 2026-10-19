//
// Created by stefano on 12/09/26.
//

#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "vk/GPUImage.h"

struct TextureStorage {
    std::vector<GPUImage> gpuImages;
    std::vector<VkSampler> samplers;
    constexpr static size_t maxTextures = 1024;

    VkDescriptorPool descPool;
    VkDescriptorSetLayout globalDSLayout;
    VkDescriptorSet globalDescSet;
};

struct Texture {
    uint32_t imageId;
    uint32_t samplerId;
};
