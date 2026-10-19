//
// Created by stefano on 11/09/26.
//

#pragma once

#include "volk.h"
#include "vk_mem_alloc.h"

struct VulkanCtx {
    VkDevice device {VK_NULL_HANDLE};
    VmaAllocator allocator {VK_NULL_HANDLE};
};
