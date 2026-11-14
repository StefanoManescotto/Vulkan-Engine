#include "toneMapPass.h"
#include "utils.h"
#include "vk/GPUImageSystem.h"
#include "vk/pipelineManager.h"
#include <shaderc/shaderc.h>
#include <vulkan/vulkan_core.h>

void ToneMapPass::init(const Device* device, ShaderManager& sManager, PipelineManager& pManager, VkDescriptorSetLayout dsLayout){
    VkPipelineLayout pLayout;
    DescriptorConfig dsConfigs {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .count = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    // m_dsSet.createDescriptorSetsImages(device->handle(), 1, dsConfigs);
    // std::array<VkDescriptorSetLayout, 1> dsLayouts { m_dsSet.getDescLayout(0) };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &dsLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };
    VK_CHECK( vkCreatePipelineLayout(device->handle(), &pipelineLayoutInfo, nullptr, &pLayout) );

    std::vector<Shader> shaders;
    sManager.addShader("tone.vert", shaderc_vertex_shader, "toneVert");
    sManager.addShader("tone.frag", shaderc_fragment_shader, "toneFrag");
    shaders.push_back(sManager.getShader("toneVert"));
    shaders.push_back(sManager.getShader("toneFrag"));
    PipelineKey key {
        .shaders = shaders,
        
        .colorAttachmentFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .pLayout = pLayout,
        
        .enableBlending = false,
        .enableDepthTest = false
    };

    m_pipelineIndex = pManager.getOrCreatePipeline(key);
}

void ToneMapPass::render(PipelineManager& pManager, ToneMapContext& ctx) { 
    Pipeline& tonePipeline = pManager.getPipeline(m_pipelineIndex);

    GPUImageSystem::transitionLayout(*ctx.colorImage, ctx.cmd, VK_PIPELINE_STAGE_NONE, 0, 
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    GPUImageSystem::transitionLayout(*ctx.sceneImage, ctx.cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkClearValue clearColor { .color = {
        0.52f,  // red
        0.12f,  // green
        0.55f,  // blue
        1.0f   // alpha
    }};

    VkRenderingAttachmentInfo colorAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.colorImage->imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the image
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // keep data for presentation
        .clearValue{clearColor}
    };

    VkRenderingInfo renderingInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea {
            .offset{ .x = 0, .y = 0 },
            .extent = ctx.extent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachInfo,
        .pDepthAttachment = nullptr
    };

    // begin dynamic rendering
    vkCmdBeginRendering(ctx.cmd, &renderingInfo);
    {
        VkViewport viewport{
            .x = 0.0f,
            .y = static_cast<float>(ctx.extent.height), // Start at bottom
            .width = static_cast<float>(ctx.extent.width),
            .height = -static_cast<float>(ctx.extent.height), // Negative height flips Y
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(ctx.cmd, 0, 1, &viewport);

        VkRect2D scissor {
            .offset{ .x = 0, .y = 0 },
            .extent = ctx.extent
        };
        vkCmdSetScissor(ctx.cmd, 0, 1, &scissor);

        vkCmdBindPipeline(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tonePipeline.handle);
        // VkDescriptorSet set = m_dsSet.getDescriptorSet();
        ctx.ds->updateTextureDescriptors(0, ctx.sceneImage, &ctx.sceneSampler, 1);
        vkCmdBindDescriptorSets(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tonePipeline.pipelineLayout, 0, 1,
                                &ctx.ds->getDescriptorSet(), 0, nullptr);
        vkCmdDraw(ctx.cmd, 3, 1, 0, 0);
        // vkCmdDrawIndexedIndirect(ctx.cmd, ctx.indirectDrawBuffer->buffer, drawStart * sizeof(VkDrawIndexedIndirectCommand), drawCount, sizeof(VkDrawIndexedIndirectCommand));
    }

    vkCmdEndRendering(ctx.cmd);
}

void ToneMapPass::destroyRenderSystem(){
    
}
