//
// Created by stefano on 12/09/26.
//

#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>
#include <vk/GPUImage.h>

struct VulkanCtx;

namespace GPUImageSystem {
    void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
    void allocateImage(const VulkanCtx &ctx, const GPUImageConfig &config, GPUImage &gpuImage);
    void destroyImage(const VulkanCtx &ctx, GPUImage &gpuImage);

    void transitionLayout(GPUImage &gpuImage, VkCommandBuffer cmd, VkPipelineStageFlagBits2 srcStage, VkAccessFlagBits2 srcAccess, VkPipelineStageFlagBits2 dstStage,
        VkAccessFlagBits2 dstAccess, VkImageLayout newLayout);
}
