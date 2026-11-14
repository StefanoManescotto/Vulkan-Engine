#include "imguiPass.h"

void ImGuiPass::render(GuiPassCtx ctx){
    VkRenderingAttachmentInfo colorAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.colorImage->imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        // .clearValue{clearColor}
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

    ctx.guiManager->beginFrame(*ctx.scene, ctx.nDraws);
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
        ctx.guiManager->render(ctx.cmd);
        
    }

    vkCmdEndRendering(ctx.cmd);
}
