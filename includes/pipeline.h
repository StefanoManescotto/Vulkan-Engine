//
// Created by stefano on 28/07/26.
//
#pragma once

#include <vector>
#include <volk.h>

struct PipelineConfig {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    VkPipelineViewportStateCreateInfo viewportInfo{};

    // Fixed-function states
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    VkPipelineRenderingCreateInfo renderingInfo{};

    // Dynamic states (viewport/scissor usually set dynamically)
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};

    VkPushConstantRange pushConstantRange{};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};


    // Pipeline layout & RenderPass references
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    uint32_t subpass = 0;
};

class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline();

    // void createPipeline(VkDevice device, const PipelineConfig& config);
    void createPipeline(VkDevice device, const PipelineConfig& config);

    [[nodiscard]] VkPipeline getPipeline() const { return m_pipeline; };
    static void getDefaultConfigs(PipelineConfig& config);
    [[nodiscard]] VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; };

    void destroyPipeline();
private:
    VkPipeline m_pipeline = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;

    VkDevice m_device = nullptr;
};
