//
// Created by stefano on 10/08/26.
//

#pragma once

#include <functional>
#include <volk.h>

class CommandContext {
public:
    void init(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex);
    void destroy();

    void immediateSubmit(std::function<void(VkCommandBuffer)>&& function);

    void submitTransientCommandBuffer(VkCommandBuffer commandBuffer);
    VkCommandBuffer startTransientCommandBuffer();

private:
    VkDevice m_device;
    VkQueue m_queue;
    VkCommandPool m_immediatePool;

    VkCommandBuffer m_immediateCmd;
    VkFence m_immediateFence;
};
