#include "toneMapPass.h"
#include "utils.h"
#include "vk/pipelineManager.h"
#include <shaderc/shaderc.h>
#include <vulkan/vulkan_core.h>
#include <array>

void ToneMapPass::init(const Device* device, ShaderManager& sManager, PipelineManager& pManager){
    VkPipelineLayout pLayout;
    DescriptorConfig dsConfigs {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .count = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    m_dsSet.createDescriptorSetsImages(device->handle(), 1, dsConfigs);
    std::array<VkDescriptorSetLayout, 1> dsLayouts { m_dsSet.getDescLayout(0) };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = dsLayouts.size(),
        .pSetLayouts = dsLayouts.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };
    VK_CHECK( vkCreatePipelineLayout(device->handle(), &pipelineLayoutInfo, nullptr, &pLayout) );

    std::vector<Shader> shaders;
    sManager.addShader("tone.frag", shaderc_fragment_shader, "tone");
    shaders.push_back(sManager.getShader("tone"));
    PipelineKey key {
        .shaders = shaders,
        
        .colorAttachmentFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .pLayout = pLayout,
        // .depthAttachmentFormat = ,
        
        .enableBlending = false,
        .enableDepthTest = false
    };

    uint32_t pIndex = pManager.getOrCreatePipeline(key);
    tonePipeline = &pManager.getPipeline(pIndex);
}

void ToneMapPass::render(PipelineManager& pManager, ToneMapContext& ctx) { 
    std::vector<VkImageMemoryBarrier2> layoutBarriers {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_NONE,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = ctx.colorImage.image,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        }
    };
    VkDependencyInfo depInfo {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(layoutBarriers.size()),
        .pImageMemoryBarriers = layoutBarriers.data()
    };
    vkCmdPipelineBarrier2(ctx.cmd, &depInfo);

    VkClearValue clearColor { .color = {
        0.12f,  // red
        0.12f,  // green
        0.55f,  // blue
        1.0f   // alpha
    }};

    VkRenderingAttachmentInfo colorAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.colorImage.imageView,
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


        int i = 0;

        // Pipeline* currentPipeline;
        vkCmdBindPipeline(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tonePipeline->handle);
        VkDescriptorSet set = m_dsSet.getDescriptorSet();
        vkCmdBindDescriptorSets(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, tonePipeline->pipelineLayout, 0, 1,
                                &m_dsSet.getDescriptorSet(), 0, nullptr);

        // vkCmdDrawIndexedIndirect(ctx.cmd, ctx.indirectDrawBuffer->buffer, drawStart * sizeof(VkDrawIndexedIndirectCommand), drawCount, sizeof(VkDrawIndexedIndirectCommand));
    }

    vkCmdEndRendering(ctx.cmd);
}
