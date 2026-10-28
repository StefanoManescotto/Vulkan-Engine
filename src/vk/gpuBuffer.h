#pragma once

#include "vk/VulkanContext.h"
#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>

struct GPUBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    uint64_t deviceAddress = 0;
};

struct GPUBufferVec {
    std::vector<GPUBuffer> buffers;
    uint32_t vertexBufferId;
    uint32_t indexBufferId;
    uint32_t matBufferId;
};

namespace gpuBuffer {
    GPUBuffer createBuffer(VulkanCtx ctx, uint64_t size, VkBufferUsageFlags bufferUsageFlagBits, VmaMemoryUsage memoryUsage, bool mappable);
    void mapCopyBufferData(VmaAllocator allocator, const GPUBuffer &buffer, size_t bufferOffset, void*  data, size_t byteSize);
}
