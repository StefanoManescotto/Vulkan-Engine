//
// Created by stefano on 24/07/26.
//
#pragma once

// #include <vulkan/vulkan.h>
#include <vector>
#include <volk.h>

struct SwapchainConfig {
    VkExtent2D configExtent {800, 600};
    VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkColorSpaceKHR imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    uint32_t imageArrayLayers = 1;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkSurfaceTransformFlagBitsKHR preTrasform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    uint32_t layerCount = 1;
    uint32_t levelCount = 1;
};

struct VmaAllocator_T;
typedef VmaAllocator_T* VmaAllocator;
struct VmaAllocation_T;
typedef VmaAllocation_T* VmaAllocation;

class Swapchain {
public:
    Swapchain() = default;
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;
    void create(VkDevice device, VkPhysicalDevice physicalDevice, VmaAllocator allocator);

    VkSwapchainKHR handle();
    VkSwapchainKHR* pHandle();
    VkSwapchainKHR createSwapchain(VkSurfaceKHR surface, const SwapchainConfig& config);
    VkSwapchainKHR recreateSwapchain(VkSurfaceKHR surface, const SwapchainConfig& config);
    void destroySwapchain();

    [[nodiscard]] VkExtent2D getSwapchainExtent() const;
    [[nodiscard]] VkImage getSwapchainImage(size_t imageIndex) const;
    [[nodiscard]] VkImageView getSwapchainImageView(size_t imageIndex) const;
    [[nodiscard]] VkSemaphore getRenderSemaphore(size_t imageIndex) const;
    [[nodiscard]] const VkSemaphore* getPRenderSemaphore(size_t imageIndex) const;
private:
    constexpr static VkFormat swapchainFormat{VK_FORMAT_B8G8R8A8_SRGB};

    VkDevice m_device = nullptr;
    VmaAllocator m_allocator = nullptr;
    VkPhysicalDevice m_physicalDevice = nullptr;

    VkSwapchainKHR m_swapchain = nullptr;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    VkExtent2D m_swapchainExtent;

    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};
