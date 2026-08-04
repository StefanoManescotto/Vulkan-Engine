//
// Created by stefano on 28/07/26.
//

#include "renderer.h"

#include <stdexcept>

#include "device.h"
#include "gameObject.h"
#include "scene.h"
#include "window.h"

void Renderer::init(const Device* device, const VmaAllocator* allocator, const Window* window) {
    m_device = device;
    m_allocator = allocator;

    swapchain.create(m_device->handle(), m_device->getPhysicalDevice(), *allocator);
    swapchain.createSwapchain(window->getSurface(), config);

    initImages();

    FrameResources res = m_frameResources[0];
    if (!m_device->supportSwapchainFormat(config.imageFormat)) {
        throw std::runtime_error("Requested swapchain format is not supported by the surface");
    }
    if (!m_device->supportFormat(res.depthImage.getConfig().format, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        throw std::runtime_error("Requested depth format is not supported by the device");
    }

    createSyncResources();
    m_shaderManager.init(m_device->handle());

    m_shaderManager.addShader("shader.vert", shaderc_vertex_shader, "vertexShader");
    m_shaderManager.addShader("shader.frag", shaderc_fragment_shader, "fragmentShader");
    MainPipelineContext pCtx {
    res.colorImage.getConfig().format,
    res.depthImage.getConfig().format,
        m_shaderManager.getShaderModule("vertexShader"),
        m_shaderManager.getShaderModule("fragmentShader")
    };
    m_mainRenderSystem.init(m_device->handle(), pCtx);

    createCommandBuffer();
}

void Renderer::initImages() {
    ImageConfig imgConfig;
    imgConfig.extent = swapchain.getSwapchainExtent();

    for (FrameResources& res : m_frameResources) {
        res.colorImage.allocateImage(m_device->handle(), m_allocator, ImageConfig::ColorAttachment(swapchain.getSwapchainExtent(), VK_FORMAT_B8G8R8A8_SRGB));
        res.depthImage.allocateImage(m_device->handle(), m_allocator, ImageConfig::DepthAttachment(swapchain.getSwapchainExtent(), VK_FORMAT_D32_SFLOAT));
    }
}

bool Renderer::beginFrame(const Window* window) {
    if (requireSwapchainRecreate) {
        config.configExtent = {.width = window->getWidth(), .height = window->getHeight()};
        swapchain.recreateSwapchain(window->getSurface(), config);
        requireSwapchainRecreate = false;
    }

    const uint32_t frameResIndex = frameIndex++ % MaxFramesInFlight;
    const uint64_t signalValue = nextSignalValue++;
    const uint64_t waitValue = signalValue - MaxFramesInFlight;

    VkSemaphoreWaitInfo waitInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &timelineSemaphore,
        .pValues = &waitValue
    };
    vkWaitSemaphores(m_device->handle(), &waitInfo, UINT64_MAX);

    // now it's safe to start recording commands
    currentFrameRes = m_frameResources[frameResIndex];
    currentFrameRes.signalValue = signalValue;
    vkResetCommandPool(m_device->handle(), currentFrameRes.commandPool, 0);

    VkSemaphore imageAcquireSemaphore = currentFrameRes.imageAcquiredSemaphore;

    VkResult acquireResult = vkAcquireNextImageKHR(m_device->handle(), swapchain.handle(), UINT64_MAX, imageAcquireSemaphore, VK_NULL_HANDLE,
                                                   &currentFrameRes.imageIndex);

    // handle resize and out-of-date images, may need swapchain recreate
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        requireSwapchainRecreate = true;
        return false;
    } else if (acquireResult == VK_SUBOPTIMAL_KHR) {
        // can render this frame, recreate next time around
        requireSwapchainRecreate = true;
    }
    return true;
}

void Renderer::endFrame() {
    VkSemaphoreSubmitInfo imageAcquireWaitInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = currentFrameRes.imageAcquiredSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT // wait before drawing to image
    };

    // signal that the image can be presented
    std::vector<VkSemaphoreSubmitInfo> semaphoreSignals {
        {
            // render work completion signal
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = swapchain.getRenderSemaphore(currentFrameRes.imageIndex),
            .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
        },
        {
            // entire frame is completed (timeline)
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = timelineSemaphore,
            .value = currentFrameRes.signalValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
        }
    };
    VkCommandBufferSubmitInfo cmdSubmitInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = currentFrameRes.commandBuffer,
    };
    VkSubmitInfo2 submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &imageAcquireWaitInfo, // ensure the image is ready
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSubmitInfo,
        .signalSemaphoreInfoCount = static_cast<uint32_t>(semaphoreSignals.size()),
        .pSignalSemaphoreInfos = semaphoreSignals.data()
    };
    vkQueueSubmit2(m_device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    // present the image
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = swapchain.getPRenderSemaphore(currentFrameRes.imageIndex), // render work completed semaphore
        .swapchainCount = 1,
        .pSwapchains = swapchain.pHandle(),
        .pImageIndices = &currentFrameRes.imageIndex,
        .pResults = nullptr
    };

    vkQueuePresentKHR(m_device->getGraphicsQueue(), &presentInfo);
}

void Renderer::renderFrame(const Window* window, Scene scene) {
    // Check if we need to recreate the swapchain
    if (!beginFrame(window)) {
        return;
    }
    ctx.obj = scene.cube;
    ctx.camera = scene.camera;
    VkCommandBufferBeginInfo cmdBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(currentFrameRes.commandBuffer, &cmdBeginInfo);

    ctx.cmd = currentFrameRes.commandBuffer;
    ctx.extent = swapchain.getSwapchainExtent();
    ctx.colorImage = currentFrameRes.colorImage;
    ctx.depthImage = currentFrameRes.depthImage;

    m_mainRenderSystem.render(ctx);


    VkImageMemoryBarrier2 barriers[3] = {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .image = ctx.colorImage.getImage(),
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        },
    {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image = swapchain.getSwapchainImage(currentFrameRes.imageIndex),
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        },
    };

    VkDependencyInfo presentDepInfo {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 2,
        .pImageMemoryBarriers = barriers
    };
    vkCmdPipelineBarrier2(currentFrameRes.commandBuffer, &presentDepInfo);

    VkExtent2D extent = swapchain.getSwapchainExtent();
    Image::copyImageToImage(ctx.cmd, ctx.colorImage.getImage(), swapchain.getSwapchainImage(currentFrameRes.imageIndex),
                            extent, extent);

    VkImageMemoryBarrier2 presentBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
        // nothing is waiting, but the cache is flushed and layout is transition
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = swapchain.getSwapchainImage(currentFrameRes.imageIndex),
        .subresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    presentDepInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &presentBarrier
    };
    vkCmdPipelineBarrier2(currentFrameRes.commandBuffer, &presentDepInfo);

    vkEndCommandBuffer(currentFrameRes.commandBuffer);

    endFrame();
}

void Renderer::createCommandBuffer() {
    for (FrameResources &res : m_frameResources) {
        // we'll give each frame its own pool, faster cmd buffer resets this way
        VkCommandPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = m_device->getGraphicsQueueIndex()
        };
        if (vkCreateCommandPool(m_device->handle(), &poolInfo, nullptr, &res.commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool");
        }

        // create the command buffer for this frame
        VkCommandBufferAllocateInfo cmdAllocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = res.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        if (vkAllocateCommandBuffers(m_device->handle(), &cmdAllocInfo, &res.commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Unable to allocate command buffer");
        }
    }
}

void Renderer::createSyncResources() {
    VkSemaphoreTypeCreateInfo semaphoreTypeInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = MaxFramesInFlight
    };
    VkSemaphoreCreateInfo timelineSemaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeInfo
    };
    if (vkCreateSemaphore(m_device->handle(), &timelineSemaphoreInfo, nullptr, &timelineSemaphore) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create the timeline semaphore");
    }

    for (FrameResources &res : m_frameResources) {
        // create the binary semaphores
        VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if (vkCreateSemaphore(m_device->handle(), &semaphoreInfo, nullptr, &res.imageAcquiredSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("Error creating the per-frame image-acquire semaphore");
        }
    }
}

void Renderer::destroyRenderer() {
    // wait in case resources are in use
    vkDeviceWaitIdle(m_device->handle());

    // frame / sync object cleanup
    if (timelineSemaphore) {
        vkDestroySemaphore(m_device->handle(), timelineSemaphore, nullptr);
    }
    for (auto &res : m_frameResources) {
        vkDestroySemaphore(m_device->handle(), res.imageAcquiredSemaphore, nullptr);
        vkDestroyCommandPool(m_device->handle(), res.commandPool, nullptr); // destroys buffers implicitly
        res.colorImage.destroyImage();
        res.depthImage.destroyImage();
    }

    m_mainRenderSystem.destroyRenderSystem();

    m_shaderManager.destroyShaders();

    swapchain.destroySwapchain();

}
