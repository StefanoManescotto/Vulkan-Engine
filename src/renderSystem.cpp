/*
//
// Created by stefano on 28/07/26.
//

#include "renderSystem.h"


void RenderSystem::render(RenderContext& ctx) {
    VkRenderingAttachmentInfo colorAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.color,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the image
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // keep data for presentation
        .clearValue{.color{0.01f, 0.01f, 0.01f, 1}}
    };
    VkRenderingAttachmentInfo depthAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.depth,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the depth data
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // don't care after rendering
        .clearValue{.depthStencil{1.0f, 0}}
    };
    VkRenderingInfo renderingInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea
        {
            .offset{.x = 0, .y = 0},
            .extent = ctx.extent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachInfo,
        .pDepthAttachment = &depthAttachInfo
    };

    // begin dynamic rendering
    vkCmdBeginRendering(ctx.cmd, &renderingInfo);
    {
        // set the viewport and scissor state
        VkViewport viewport {
            .x = 0, .y = 0,
            .width = static_cast<float>(ctx.extent.width),
            .height = static_cast<float>(ctx.extent.height)
        };
        vkCmdSetViewport(ctx.cmd, 0, 1, &viewport);

        VkRect2D scissor {
            .offset{.x = 0, .y = 0},
            .extent = ctx.extent
        };
        vkCmdSetScissor(ctx.cmd, 0, 1, &scissor);

        // draw our triangle
        vkCmdBindPipeline(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());
        vkCmdDraw(ctx.cmd, 3, 1, 0, 0);
    }
    // end dynamic rendering
    vkCmdEndRendering(ctx.cmd);
}
*/
