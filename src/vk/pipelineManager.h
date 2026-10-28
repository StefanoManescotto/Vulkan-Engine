#pragma once

#include "pipeline.h"
#include "render/frameLayout.h"
#include "render/shaderManager.h"

#include <unordered_map>
#include <vector>
#include "render/frameLayout.h"

struct PipelineKey {
    VkShaderModule vertShader;
    VkShaderModule fragShader;
    
    VkFormat colorAttachmentFormat;
    VkFormat depthAttachmentFormat;
    
    bool enableBlending = false;
    bool enableDepthTest = true;

    bool operator==(const PipelineKey& other) const {
        return vertShader == other.vertShader &&
            fragShader == other.fragShader &&
            colorAttachmentFormat == other.colorAttachmentFormat &&
            depthAttachmentFormat == other.depthAttachmentFormat &&
            enableBlending == other.enableBlending &&
            enableDepthTest == other.enableDepthTest;
    }
};

struct PipelineKeyHash {
    std::size_t operator()(const PipelineKey& key) const noexcept {
        std::size_t seed = 0;

        auto hash_combine = [&seed](std::size_t h) {
            seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };

        hash_combine(std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(key.vertShader)));
        hash_combine(std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(key.fragShader)));

        hash_combine(std::hash<int>{}(static_cast<int>(key.colorAttachmentFormat)));
        hash_combine(std::hash<int>{}(static_cast<int>(key.depthAttachmentFormat)));

        hash_combine(std::hash<bool>{}(key.enableBlending));
        hash_combine(std::hash<bool>{}(key.enableDepthTest));

        return seed;
    }
};

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


class PipelineManager {
public:
    void init(VkDevice device){ m_device = device; }
    uint32_t createPipeline(const PipelineConfig& config,  const PipelineKey& key, FrameLayout& frameLayout);
    uint32_t getOrCreatePipeline(const PipelineKey& key, FrameLayout& frameLayout);
    Pipeline& getPipeline(int index) { return m_pipelines.at(index); }
    void destroyPipelines();


    
    static void constexpr getDefaultConfigs(PipelineConfig& configInfo) {
        // --- Input Assembly ---
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // --- Viewport State (Configured as Dynamic State) ---
        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = nullptr; // Handled dynamically at render time
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;  // Handled dynamically at render time

        // --- Rasterizer ---
        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;

        // --- Multisampling ---
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // --- Color Blending ---
        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;

        // --- Depth/Stencil ---
        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;

        // --- Render Info ---
        configInfo.renderingInfo.pNext = nullptr;
        configInfo.renderingInfo.colorAttachmentCount = 0;
        configInfo.renderingInfo.pColorAttachmentFormats = nullptr;
        configInfo.renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

        // --- Dynamic States ---
        configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data(); // Safe internal vector
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    }

private:
    std::vector<Pipeline> m_pipelines;
    std::unordered_map<PipelineKey, uint32_t, PipelineKeyHash> m_lookup;
    VkDevice m_device = nullptr;
    ShaderManager shaderManager;
};
