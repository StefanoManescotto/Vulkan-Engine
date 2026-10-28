//
// Created by stefano on 10/08/26.
//

#include "vk/commandContext.h"

#include <stdexcept>

void CommandContext::init(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex) {
    m_device = device;
    m_queue = queue;

    VkCommandPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex
    };
    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_immediatePool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_immediatePool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_immediateCmd) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers.");
    }

    VkFenceCreateInfo fenceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    if (vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_immediateFence) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create the fence.");
    }
}

void CommandContext::destroy() {
    vkDestroyCommandPool(m_device, m_immediatePool, nullptr);
    vkDestroyFence(m_device, m_immediateFence, nullptr);
}

void CommandContext::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
    vkResetFences(m_device, 1, &m_immediateFence);

    VkCommandBufferBeginInfo cmdBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(m_immediateCmd, &cmdBeginInfo);
    function(m_immediateCmd);
    vkEndCommandBuffer(m_immediateCmd);

    VkCommandBufferSubmitInfo cmdInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = m_immediateCmd
    };

    VkSubmitInfo2 submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdInfo
    };

    vkQueueSubmit2(m_queue, 1, &submitInfo, m_immediateFence);
    vkWaitForFences(m_device, 1, &m_immediateFence, VK_TRUE, UINT64_MAX);
    
    vkResetCommandBuffer(m_immediateCmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

VkCommandBuffer CommandContext::startTransientCommandBuffer() {
	// allocate the transient command buffer
	VkCommandBufferAllocateInfo cmdAllocInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_immediatePool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer = nullptr;
	if (vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Unable to allocate command buffer");
	}

	// begin the command buffer
	VkCommandBufferBeginInfo beginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        vkFreeCommandBuffers(m_device, m_immediatePool, 1, &commandBuffer);
        throw std::runtime_error("Unable to begin command buffer");
	}

	return commandBuffer;
}

void CommandContext::submitTransientCommandBuffer(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer
	};

	vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
	vkQueueWaitIdle(m_queue);
	vkFreeCommandBuffers(m_device, m_immediatePool, 1, &commandBuffer);
    
    // vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}
