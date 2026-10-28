//
// Created by stefano on 29/07/26.
//

#pragma once

#include <volk.h>
#include "vk_mem_alloc.h"

struct GPUImageConfig {
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent {.width = 800, .height = 600};
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageSubresourceRange subresource {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1, .layerCount = 1
    };

    static GPUImageConfig ColorAttachment(VkExtent2D extent, VkFormat format = VK_FORMAT_B8G8R8A8_SRGB) {
        return GPUImageConfig{
            .format = format,
            .extent = { extent.width, extent.height },
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .subresource { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        };
    }

    static GPUImageConfig DepthAttachment(VkExtent2D extent, VkFormat format = VK_FORMAT_D32_SFLOAT) {
        return GPUImageConfig{
            .format = format,
            .extent = { extent.width, extent.height },
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .subresource {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
        };
    }
};

struct GPUImage {
    VkImage image {VK_NULL_HANDLE};
    VkImageView imageView {VK_NULL_HANDLE};
    VmaAllocation allocation {VK_NULL_HANDLE};
    VkImageLayout currentLayout {VK_IMAGE_LAYOUT_UNDEFINED};
    GPUImageConfig config {};
};
