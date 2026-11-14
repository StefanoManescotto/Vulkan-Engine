//
// Created by stefano on 28/07/26.
//

#pragma once

#include "vk/GPUImage.h"
#include "vk/device.h"
#include "vk/pipelineManager.h"
#include "vk/descriptorSets.h"
#include "vk/device.h"

struct ToneMapContext {
    VkCommandBuffer cmd = nullptr;
    VkExtent2D extent = VkExtent2D(800, 600);

    GPUImage* colorImage = nullptr;
    GPUImage* sceneImage = nullptr;
    // VkSampler sceneSampler = VK_NULL_HANDLE;
    DescriptorSets* ds = nullptr;
};

class ToneMapPass {
public:
    void init(const Device* device, ShaderManager& sManager, PipelineManager& pManager, VkDescriptorSetLayout dsLayout);
    void render(PipelineManager& pManager, ToneMapContext& ctx);

    void destroyRenderSystem();

private:
    VkDevice m_device;
    uint32_t m_pipelineIndex = UINT32_MAX;
    // DescriptorSets m_ds;
    VkSampler sceneSampler;
};
