//
// Created by stefano on 12/09/26.
//

#include "GPUImageSystem.h"

#include <stdexcept>
#include <fmt/format.h>

#include "vk/VulkanContext.h"

namespace GPUImageSystem {
    void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize,
        VkExtent2D dstSize) {
        VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

        blitRegion.srcOffsets[1].x = srcSize.width;
        blitRegion.srcOffsets[1].y = srcSize.height;
        blitRegion.srcOffsets[1].z = 1;

        blitRegion.dstOffsets[1].x = dstSize.width;
        blitRegion.dstOffsets[1].y = dstSize.height;
        blitRegion.dstOffsets[1].z = 1;

        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcSubresource.mipLevel = 0;

        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstSubresource.mipLevel = 0;

        VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
        blitInfo.dstImage = destination;
        blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        blitInfo.srcImage = source;
        blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blitInfo.filter = VK_FILTER_LINEAR;
        blitInfo.regionCount = 1;
        blitInfo.pRegions = &blitRegion;

        vkCmdBlitImage2(cmd, &blitInfo);
    }

    void allocateImage(const VulkanCtx &ctx, const GPUImageConfig &config, GPUImage &gpuImage) {
        gpuImage.config = config;

        VkImageCreateInfo imageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = config.format,
            .extent { config.extent.width, config.extent.height, 1 },
            .mipLevels = config.mipLevels,
            .arrayLayers = config.arrayLayers,
            .samples = config.samples,
            .tiling = config.tiling,
            .usage = config.usage,
            .initialLayout = config.layout
        };

        VmaAllocationCreateInfo allocInfo {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        VkResult result = vmaCreateImage(ctx.allocator, &imageCreateInfo, &allocInfo, &gpuImage.image, &gpuImage.allocation, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(fmt::format("Failed to create image. VkResult = {}", static_cast<int>(result)));
        }

        VkImageViewCreateInfo imageViewInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = gpuImage.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = config.format,
            .subresourceRange = config.subresource
        };
        if (vkCreateImageView(ctx.device, &imageViewInfo, nullptr, &gpuImage.imageView) != VK_SUCCESS) {
            throw std::runtime_error("Error creating image view");
        }
    }

    void destroyImage(const VulkanCtx &ctx, GPUImage &gpuImage) {
        if (gpuImage.imageView) {
            vkDestroyImageView(ctx.device, gpuImage.imageView, nullptr);
            vmaDestroyImage(ctx.allocator, gpuImage.image, gpuImage.allocation);
            gpuImage.imageView = nullptr;
            gpuImage = {};
        }
    }

    void transitionLayout(GPUImage &gpuImage, VkCommandBuffer cmd, VkPipelineStageFlagBits2 srcStage, VkAccessFlagBits2 srcAccess,
        VkPipelineStageFlagBits2 dstStage, VkAccessFlagBits2 dstAccess, VkImageLayout newLayout) {

        VkImageMemoryBarrier2 barrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = srcStage,
                .srcAccessMask = srcAccess,
                .dstStageMask = dstStage,
                .dstAccessMask = dstAccess,
                .oldLayout = gpuImage.currentLayout,
                .newLayout = newLayout,
                .image = gpuImage.image,
                .subresourceRange = gpuImage.config.subresource
            };
            VkDependencyInfo depInfo {
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier
            };
            vkCmdPipelineBarrier2(cmd, &depInfo);

            gpuImage.currentLayout = newLayout;
    }
}
