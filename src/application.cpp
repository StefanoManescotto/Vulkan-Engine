#include "application.h"
// #include "utils.h"

#include <SDL3/SDL.h>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#define VMA_IMPLEMENTATION
#include <thirdparty/vk_mem_alloc.h>

#include <fmt/printf.h>
#include <fstream>

void Application::showError(const std::string &errorMessage) const {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errorMessage.c_str(), window.handle());
}

void Application::manageInputs() {
    if (window.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running = false;
    }
    if (window.isKeyPressed(SDL_SCANCODE_A)) {
        fmt::print("Pressed A - {} {}\n", window.getMousePosition().x, window.getMousePosition().y);
    }
    if (window.isKeyPressed(SDL_SCANCODE_S)) {
        fmt::print("Pressed S - {}\n", window.getMouseDelta());
    }

    window.updateInputState();
}

bool Application::initialize() {
    window.createWindow();

    if (!initializeVulkan()) {
        return false;
    }
    return true;
}

bool Application::initializeVulkan() {
    config.configExtent = {.width = width, .height = height};
    Pipeline::getDefaultConfigs(pipelineConfig);

    if (!createVulkanInstance()) {
        showError("Couldn't create a vulkan instance");
        return false;
    }

    window.createSurface(vulkanInstance);

    device.create(window.getSurface());
    device.findPhysicalDevice(vulkanInstance, config.imageFormat);
    device.findGraphicsQueue();
    device.createDevice();

    if (!initializeVMA()) {
        showError("Unable to create Vulkan Memory Allocator");
        return false;
    }

    swapchain.create(device.handle(), device.getPhysicalDevice(), vmaAllocator);
    swapchain.createSwapchain(window.getSurface(), config);

    shader.createShaders(device.handle());

    pipeline.createPipeline(device.handle(), pipelineConfig, shader, 1, &config.imageFormat, config.depthFormat);

    if (!createSyncResources()) {
        showError("Couldn't create the sync related resources");
        return false;
    }

    if (!createCommandBuffers()) {
        showError("Couldn't create command buffer objects");
        return false;
    }

    return true;
}

void Application::shutdown() {
    // wait in case resources are in use
    vkDeviceWaitIdle(device.handle());

    // frame / sync object cleanup
    if (timelineSemaphore) {
        vkDestroySemaphore(device.handle(), timelineSemaphore, nullptr);
    }
    for (auto &res: frameResources) {
        vkDestroySemaphore(device.handle(), res.imageAcquiredSemaphore, nullptr);
        // vkDestroySemaphore(device, res.renderCompleteSemaphore, nullptr);
        vkDestroyCommandPool(device.handle(), res.commandPool, nullptr); // destroys buffers implicitly
    }

    // pipeline cleanup
    pipeline.destroyPipeline();

    // cleanup shaders
    shader.destroyShaders();

    swapchain.destroySwapchain();

    // VMA
    if (vmaAllocator) {
        vmaDestroyAllocator(vmaAllocator);
    }

    // cleanup Vulkan
    window.destroySurface();

    device.destroyDevice();

    if (vulkanInstance) {
        vkDestroyInstance(vulkanInstance, nullptr);
    }
    volkFinalize();

    window.destroyWindow();
}

void Application::run() {
    running = true;
    while (running) {
        if (!window.pollEvents()) {
            running = false;
            break;
        }
        manageInputs();

        render();
    }
}

bool Application::createVulkanInstance() {
    // Initialize Volk and load Vk function pointers
    if (volkInitialize() != VK_SUCCESS) {
        showError("Error initializing Volk");
        return false;
    }

    // Create the vulkan application instance
    VkApplicationInfo appInfo
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "My First Triangle",
        .apiVersion = VulkanVersion,
    };

    uint32_t instExtCount = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&instExtCount);

    std::vector<const char *> requestedLayers
    {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = instExtCount,
        .ppEnabledExtensionNames = extensions
    };

    if (vkCreateInstance(&instCreateInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
        return false;
    }

    volkLoadInstance(vulkanInstance);
    return true;
}

bool Application::initializeVMA() {
    VmaVulkanFunctions vmaFuncInfo{};
    VmaAllocatorCreateInfo vmaAllocInfo {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = device.getPhysicalDevice(),
        .device = device.handle(),
        .pVulkanFunctions = &vmaFuncInfo,
        .instance = vulkanInstance,
        .vulkanApiVersion = VulkanVersion
    };

    // vma can import directly from volk
    vmaImportVulkanFunctionsFromVolk(&vmaAllocInfo, &vmaFuncInfo);

    if (vmaCreateAllocator(&vmaAllocInfo, &vmaAllocator) != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool Application::createSyncResources() {
    VkSemaphoreTypeCreateInfo semaphoreTypeInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = MaxFramesInFlight
    };
    VkSemaphoreCreateInfo timelineSemaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeInfo
    };
    if (vkCreateSemaphore(device.handle(), &timelineSemaphoreInfo, nullptr, &timelineSemaphore) != VK_SUCCESS) {
        showError("Unable to create the timeline semaphore");
        return false;
    }

    for (FrameResources &res : frameResources) {
        // create the binary semaphores
        VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if (vkCreateSemaphore(device.handle(), &semaphoreInfo, nullptr, &res.imageAcquiredSemaphore) != VK_SUCCESS) {
            showError("Error creating the per-frame image-acquire semaphore");
            return false;
        }
    }

    return true;
}

bool Application::createCommandBuffers() {
    for (FrameResources &res : frameResources) {
        // we'll give each frame its own pool, faster cmd buffer resets this way
        VkCommandPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = device.getGraphicsQueueIndex()
        };
        if (vkCreateCommandPool(device.handle(), &poolInfo, nullptr, &res.commandPool) != VK_SUCCESS) {
            showError("Unable to create command buffer pool");
            return false;
        }

        // create the command buffer for this frame
        VkCommandBufferAllocateInfo cmdAllocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = res.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        if (vkAllocateCommandBuffers(device.handle(), &cmdAllocInfo, &res.commandBuffer) != VK_SUCCESS) {
            showError("Unable to allocate command buffer");
            return false;
        }
    }
    return true;
}

void Application::render() {
    if (requireSwapchainRecreate) {
        config.configExtent = {.width = width, .height = height};
        swapchain.recreateSwapchain(window.getSurface(), config);
        requireSwapchainRecreate = false;
    }

    const uint32_t frameResIndex = frameIndex++ % MaxFramesInFlight;
    const uint64_t signalValue = nextSignalValue++;
    const uint64_t waitValue = signalValue - MaxFramesInFlight;

    VkSemaphoreWaitInfo waitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &timelineSemaphore,
        .pValues = &waitValue
    };
    vkWaitSemaphores(device.handle(), &waitInfo, UINT64_MAX);

    // now it's safe to start recording commands
    FrameResources &res = frameResources[frameResIndex];
    vkResetCommandPool(device.handle(), res.commandPool, 0);

    VkSemaphore imageAcquireSemaphore = frameResources[frameResIndex].imageAcquiredSemaphore;

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(device.handle(), swapchain.handle(), UINT64_MAX, imageAcquireSemaphore, VK_NULL_HANDLE,
                                                   &imageIndex);

    // handle resize and out-of-date images, may need swapchain recreate
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        requireSwapchainRecreate = true;
        return;
    } else if (acquireResult == VK_SUBOPTIMAL_KHR) {
        // can render this frame, recreate next time around
        requireSwapchainRecreate = true;
    }

    // begin recording commands
    VkCommandBufferBeginInfo cmdBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(res.commandBuffer, &cmdBeginInfo);

    // transition the color and depth images
    std::vector<VkImageMemoryBarrier2> layoutBarriers
    {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_NONE,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = swapchain.getSwapchainImage(imageIndex),
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        },
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_NONE,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            // both specified to control memory access at both stages (write)
            .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .image = swapchain.getDepthImage(),
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        }
    };
    VkDependencyInfo depInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(layoutBarriers.size()),
        .pImageMemoryBarriers = layoutBarriers.data()
    };
    vkCmdPipelineBarrier2(res.commandBuffer, &depInfo);

    // set up the attachments (color and depth) and begin rendering (dynamic)
    VkRenderingAttachmentInfo colorAttachInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain.getSwapchainImageView(imageIndex),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the image
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // keep data for presentation
        .clearValue{.color{0.01f, 0.01f, 0.01f, 1}}
    };
    VkRenderingAttachmentInfo depthAttachInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain.getDepthImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the depth data
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // don't care after rendering
        .clearValue{.depthStencil{1.0f, 0}}
    };
    VkRenderingInfo renderingInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea
        {
            .offset{.x = 0, .y = 0},
            .extent = swapchain.getSwapchainExtent()
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachInfo,
        .pDepthAttachment = &depthAttachInfo
    };

    // begin dynamic rendering
    vkCmdBeginRendering(res.commandBuffer, &renderingInfo);
    {
        // set the viewport and scissor state
        VkViewport viewport {
            .x = 0, .y = 0,
            .width = static_cast<float>(swapchain.getSwapchainExtent().width),
            .height = static_cast<float>(swapchain.getSwapchainExtent().height)
        };
        vkCmdSetViewport(res.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor {
            .offset{.x = 0, .y = 0},
            .extent = swapchain.getSwapchainExtent()
        };
        vkCmdSetScissor(res.commandBuffer, 0, 1, &scissor);

        // draw our triangle
        vkCmdBindPipeline(res.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());
        vkCmdDraw(res.commandBuffer, 3, 1, 0, 0);
    }
    // end dynamic rendering
    vkCmdEndRendering(res.commandBuffer);

    // transition the image from color attachment to presentation so we can show it
    VkImageMemoryBarrier2 presentLayoutBarrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
        // nothing is waiting, but the cache is flushed and layout is transition
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = swapchain.getSwapchainImage(imageIndex),
        .subresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    VkDependencyInfo presentDepInfo {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &presentLayoutBarrier
    };
    vkCmdPipelineBarrier2(res.commandBuffer, &presentDepInfo);

    vkEndCommandBuffer(res.commandBuffer);

    // ensure swapchain image is actually available to start color outp ut
    VkSemaphoreSubmitInfo imageAcquireWaitInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = imageAcquireSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT // wait before drawing to image
    };
    // signal that the image can be presented
    std::vector<VkSemaphoreSubmitInfo> semaphoreSignals {
        {
            // render work completion signal
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = swapchain.getRenderSemaphore(imageIndex),
            .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
        },
        {
            // entire frame is completed (timeline)
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = timelineSemaphore,
            .value = signalValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
        }
    };
    VkCommandBufferSubmitInfo cmdSubmitInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = res.commandBuffer,
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
    vkQueueSubmit2(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    // present the image
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = swapchain.getPRenderSemaphore(imageIndex), // render work completed semaphore
        .swapchainCount = 1,
        .pSwapchains = swapchain.pHandle(),
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    vkQueuePresentKHR(device.getGraphicsQueue(), &presentInfo);
}
