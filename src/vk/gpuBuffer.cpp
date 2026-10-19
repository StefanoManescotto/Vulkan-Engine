#include "vk/gpuBuffer.h"
#include "vk/VulkanContext.h"

#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

// #include "utils.h"

namespace gpuBuffer {
    GPUBuffer createBuffer(VulkanCtx ctx, uint64_t size, VkBufferUsageFlags bufferUsageFlag, VmaMemoryUsage memoryUsage, bool mappable) {
        GPUBuffer gpuBuffer;

        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .size = size,
            .usage = bufferUsageFlag,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocationCreateInfo = {
            .flags = mappable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0u,
            .usage = memoryUsage,
        };

        // VK_CHECK( vmaCreateBuffer(ctx.allocator, &bufferCreateInfo, &allocationCreateInfo,
        //     &gpuBuffer.buffer, &gpuBuffer.allocation, nullptr) );

        if (vmaCreateBuffer(ctx.allocator, &bufferCreateInfo, &allocationCreateInfo,
            &gpuBuffer.buffer, &gpuBuffer.allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer.");
        }
        
        // BDA send device pointer
        if (bufferUsageFlag & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT){
            VkBufferDeviceAddressInfo vertBdaInfo {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = gpuBuffer.buffer
            };
            gpuBuffer.deviceAddress = vkGetBufferDeviceAddress(ctx.device, &vertBdaInfo);
        }

        return gpuBuffer;
    }

    void mapCopyBufferData(VmaAllocator allocator, const GPUBuffer &buffer, size_t bufferOffset, void*  data, size_t byteSize) {
        // map and write buffer data
        void *buffPtr = nullptr;
        if (vmaMapMemory(allocator, buffer.allocation, &buffPtr) != VK_SUCCESS) {
            throw std::runtime_error("Unable to map buffer memory");
            return;
        }
        std::memcpy(static_cast<char *>(buffPtr) + bufferOffset, data, byteSize);
        vmaUnmapMemory(allocator, buffer.allocation);
    }
}
