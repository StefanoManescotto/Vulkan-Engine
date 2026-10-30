#pragma once

#include <volk.h>
#include <vulkan/vulkan_core.h>

#include "vk/device.h"
#include "vk/swapchain.h"

struct VulkanCore {
    Device& device;
    VkInstance instance;
    Swapchain& swapchain;
};
