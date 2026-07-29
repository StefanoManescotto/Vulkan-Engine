//
// Created by stefano on 29/07/26.
//

#pragma once

#include <volk.h>
#include "thirdparty/vk_mem_alloc.h"

struct ImageConfig {
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

    static ImageConfig ColorAttachment(VkExtent2D extent, VkFormat format = VK_FORMAT_B8G8R8A8_SRGB) {
        return ImageConfig{
            .format = format,
            .extent = { extent.width, extent.height },
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .subresource { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        };
    }

    static ImageConfig DepthAttachment(VkExtent2D extent, VkFormat format = VK_FORMAT_D32_SFLOAT) {
        return ImageConfig{
            .format = format,
            .extent = { extent.width, extent.height },
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .subresource {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
        };
    }
};

class Image {
public:
    void allocateImage(VkDevice device, const VmaAllocator* allocator, ImageConfig config);
    void destroyImage();

    [[nodiscard]] VkImage getImage() const { return m_image; }
    [[nodiscard]] VkImageView getImageView() const { return m_imageView; }
    [[nodiscard]] ImageConfig getConfig() const { return m_config; }

    static void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

private:
    VkImage m_image = nullptr;
    VkImageView m_imageView = nullptr;
    ImageConfig m_config;

    VkDevice m_device = nullptr;
    VmaAllocation m_allocation = nullptr;
    const VmaAllocator* m_allocator = nullptr;
};

