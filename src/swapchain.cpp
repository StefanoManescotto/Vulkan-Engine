//
// Created by stefano on 24/07/26.
//

#include "swapchain.h"

#include <stdexcept>
#include <thirdparty/vk_mem_alloc.h>

#include "application.h"

Swapchain::~Swapchain() {
    destroySwapchain();
}

VkSwapchainKHR Swapchain::handle() {
    return m_swapchain;
}

VkSwapchainKHR* Swapchain::pHandle() {
    return &m_swapchain;
}

void Swapchain::create(VkDevice device, VkPhysicalDevice physicalDevice, VmaAllocator allocator) {
    m_physicalDevice = physicalDevice;
    m_device = device;
    m_allocator = allocator;
}

VkSwapchainKHR Swapchain::createSwapchain(VkSurfaceKHR surface, const SwapchainConfig& config) {
    m_swapchainExtent = config.configExtent;

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("m_physicalDevice is VK_NULL_HANDLE in Swapchain::create!");
    }

    if (surface == VK_NULL_HANDLE) {
        throw std::runtime_error("surface is VK_NULL_HANDLE passed into Swapchain::create!");
    }

    // Make sure surfaceCaps is declared properly on the stack
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, surface, &surfaceCaps) != VK_SUCCESS) {
        throw std::runtime_error("Couldn't get the surface capabilities");
    }

    uint32_t imageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0 && surfaceCaps.maxImageCount < imageCount) {
        imageCount = surfaceCaps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = config.imageFormat,
        .imageColorSpace = config.imageColorSpace,
        .imageExtent = config.configExtent,
        .imageArrayLayers = config.imageArrayLayers,
        .imageUsage = config.imageUsage,
        .preTransform = config.preTrasform,
        .compositeAlpha = config.compositeAlpha,
        .presentMode = config.presentMode
    };

    if (vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Error creating swapchain");
    }

    // grab the swapchain images
    // imageCount could be different from what is set above in .minImageCount
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
    m_swapchainImageViews.resize(imageCount);

    m_renderFinishedSemaphores.resize(imageCount);

    // create the swapchain image views and semaphores signaling the ending of the render
    for (size_t i = 0; i < m_swapchainImages.size(); ++i) {
        VkImageViewCreateInfo imgViewInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = config.imageFormat,
            .subresourceRange {
                .aspectMask = config.aspectMask,
                .levelCount = config.levelCount,
                .layerCount = config.layerCount
            }
        };

        if (vkCreateImageView(m_device, &imgViewInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error creating swapchain image view");
        }

        VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error creating the render finished semaphore");
        }
    }

    createDepthImage(config);

    return m_swapchain;
}

void Swapchain::createDepthImage(SwapchainConfig config) {
    VkImageCreateInfo depthCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = config.depthFormat,
        .extent{.width = m_swapchainExtent.width, .height = m_swapchainExtent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocInfo {
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    if (vmaCreateImage(m_allocator, &depthCreateInfo, &allocInfo, &m_depthImage, &m_depthAllocation, nullptr) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error allocating depth image");
    }

    VkImageViewCreateInfo depthImgViewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = config.depthFormat,
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
    };
    if (vkCreateImageView(m_device, &depthImgViewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
        throw std::runtime_error("Error creating depth image view");
    }
}

VkSwapchainKHR Swapchain::recreateSwapchain(VkSurfaceKHR surface, const SwapchainConfig& config) {
    vkDeviceWaitIdle(m_device);
    destroySwapchain();
    createSwapchain(surface, config);
    return m_swapchain;
}

void Swapchain::destroySwapchain() {
    for (VkImageView swapchainImgView : m_swapchainImageViews) {
        if (swapchainImgView)
            vkDestroyImageView(m_device, swapchainImgView, nullptr);
    }
    m_swapchainImageViews.clear();

    for (VkSemaphore semaphore : m_renderFinishedSemaphores) {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }
    m_renderFinishedSemaphores.clear();

    if (m_swapchain) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = nullptr;
    }

    if (m_depthImageView) {
        vkDestroyImageView(m_device, m_depthImageView, nullptr);
        vmaDestroyImage(m_allocator, m_depthImage, m_depthAllocation);
        m_depthImageView = nullptr;
    }
}

VkExtent2D Swapchain::getSwapchainExtent() const {
    return m_swapchainExtent;
}

VkImage Swapchain::getSwapchainImage(size_t imageIndex) const {
    return m_swapchainImages[imageIndex];
}

VkImageView Swapchain::getSwapchainImageView(size_t imageIndex) const {
    return m_swapchainImageViews[imageIndex];
}

VkImage Swapchain::getDepthImage() const {
    return m_depthImage;
}

VkImageView Swapchain::getDepthImageView() const {
    return m_depthImageView;
}

VkSemaphore Swapchain::getRenderSemaphore(size_t imageIndex) const {
    return m_renderFinishedSemaphores[imageIndex];
}

const VkSemaphore* Swapchain::getPRenderSemaphore(size_t imageIndex) const {
    return &m_renderFinishedSemaphores[imageIndex];
}
