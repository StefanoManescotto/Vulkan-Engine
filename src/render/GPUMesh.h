//
// Created by stefano on 11/09/26.
//

#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

struct GPUMesh {
    VkBuffer vertexBuffer {VK_NULL_HANDLE};
    VmaAllocation allocation {VK_NULL_HANDLE};

    VkDeviceSize vertexBufferSize {0};
    VkDeviceSize indexBufferSize {0};
    uint32_t indexCount {0};

    std::uint64_t vertexBufferAddress {0};
    std::uint64_t indexBufferAddress {0};
};
