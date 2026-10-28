#include "pipelineManager.h"
#include "render/frameLayout.h"
#include "vk/pipeline.h"
#include <fmt/base.h>
#include <stdexcept>

uint32_t PipelineManager::getOrCreatePipeline(const PipelineKey& key, FrameLayout& frameLayout) {
    auto it = m_lookup.find(key);
    if (it != m_lookup.end()) {
        return it->second;
    }

    PipelineConfig pipelineConfig;
    getDefaultConfigs(pipelineConfig);

    pipelineConfig.shaderStages = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = key.vertShader,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = key.fragShader,
            .pName = "main"
        }
    };

    pipelineConfig.renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &key.colorAttachmentFormat,
        .depthAttachmentFormat = key.depthAttachmentFormat,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };
    
    return createPipeline(pipelineConfig, key, frameLayout);
}

uint32_t PipelineManager::createPipeline(const PipelineConfig& config, const PipelineKey& key, FrameLayout& frameLayout) {
    Pipeline newPipeline;

    VkPipelineVertexInputStateCreateInfo vertInputInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = config.inputAssemblyInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = config.depthStencilInfo;
    VkPipelineViewportStateCreateInfo viewportInfo = config.viewportInfo;
    VkPipelineRasterizationStateCreateInfo rasterInfo = config.rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multiSampleInfo = config.multisampleInfo;
    VkPipelineColorBlendStateCreateInfo blendInfo = config.colorBlendInfo;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = config.dynamicStateInfo; // Enable dynamic state

    // structure required for dynamic rendering
    VkPipelineRenderingCreateInfo renderInfo = config.renderingInfo;

    // Create the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderInfo,
        .stageCount = static_cast<uint32_t>(config.shaderStages.size()),
        .pStages = config.shaderStages.data(),
        .pVertexInputState = &vertInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterInfo,
        .pMultisampleState = &multiSampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &blendInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = frameLayout.defaultPipelineLayout,
        .renderPass = VK_NULL_HANDLE,
    };
    if (vkCreateGraphicsPipelines(m_device, nullptr, 1, &pipelineInfo, nullptr, &newPipeline.handle) != VK_SUCCESS) {
        throw std::runtime_error("Error creating the pipeline");
    }

    newPipeline.pipelineLayout = frameLayout.defaultPipelineLayout;
    
    uint32_t index = static_cast<uint32_t>(m_pipelines.size());
    m_pipelines.push_back(newPipeline);
    m_lookup[key] = index;

    return m_pipelines.size() - 1;
}

void PipelineManager::destroyPipelines() {
    for (Pipeline& p : m_pipelines) {
        if (p.pipelineLayout) {
            vkDestroyPipelineLayout(m_device, p.pipelineLayout, nullptr);
            p.pipelineLayout = nullptr;
        }
        if (p.handle) {
            vkDestroyPipeline(m_device, p.handle, nullptr);
            p.handle = nullptr;
        }
    }
    m_pipelines.clear();
}
