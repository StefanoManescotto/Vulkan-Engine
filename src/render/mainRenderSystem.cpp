//
// Created by stefano on 28/07/26.
//

#include "mainRenderSystem.h"

#include <iostream>
#include <glm/fwd.hpp>

struct MeshPushConstants {
    uint64_t vertexBufferAddress; // Direct BDA address to vertices
    uint64_t _padding;
    // uint32_t textureIndex;
    glm::vec4 color;
    glm::mat4 modelMatrix;
};

void MainRenderSystem::init(VkDevice device, MainPipelineContext& pipelineCtx) {
    Pipeline::getDefaultConfigs(pipelineConfig);
    pipelineConfig.renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &pipelineCtx.colorFormat,
        .depthAttachmentFormat = pipelineCtx.depthFormat,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    pipelineConfig.pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // Stages accessing the PC
        .offset = 0,
        .size = sizeof(MeshPushConstants)
    };

    // need to define a pipeline layout
    pipelineConfig.pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pipelineConfig.pushConstantRange
    };

    createPipeline(device, pipelineCtx);
}

void MainRenderSystem::render(MainRenderContext& ctx) {

    std::vector<VkImageMemoryBarrier2> layoutBarriers {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_NONE,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = ctx.colorImage.getImage(),
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        },
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_NONE,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            // both specified to control memory access at both stages (write)
            .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .image = ctx.depthImage.getImage(),
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        }
    };
    VkDependencyInfo depInfo
    {
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

    // std::cout << sizeof(Vertex) << '\n';

    // std::cout << offsetof(Vertex, pos) << '\n';
    // std::cout << offsetof(Vertex, normal) << '\n';
    // std::cout << offsetof(Vertex, uv) << '\n';

    VkRenderingAttachmentInfo colorAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.colorImage.getImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the image
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // keep data for presentation
        .clearValue{clearColor}
    };
    VkRenderingAttachmentInfo depthAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.depthImage.getImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear the depth data
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // don't care after rendering
        .clearValue{ .depthStencil{ 1.0f, 0 } }
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
        .pDepthAttachment = &depthAttachInfo
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

        vkCmdBindPipeline(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());




        MeshPushConstants pc{
            .vertexBufferAddress = ctx.obj.meshes.at(0).cubeBufferAddress,
            .color               = glm::vec4(1.0f, 0.5f, 0.2f, 1.0f),
            .modelMatrix = ctx.obj.meshes.at(0).calculateModelMatrix(ctx.camera)
        };

        // Push BDA address to GPU
        vkCmdPushConstants(
            ctx.cmd,
            pipeline.getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(pc),
            &pc
        );

        // VkDeviceSize vertexOffset = 0;
        // Bind index portion using the offset (vBufSize)
        // vkCmdBindVertexBuffers(ctx.cmd, 0, 1, &ctx.obj.meshes.at(0).vertexBuffer, &vertexOffset);
        vkCmdBindIndexBuffer(ctx.cmd, ctx.obj.meshes.at(0).vertexBuffer,  ctx.obj.meshes.at(0).buffSize, VK_INDEX_TYPE_UINT16);

        // Draw indexed
        vkCmdDrawIndexed(ctx.cmd, 36, 1, 0, 0, 0);

        // vkCmdDraw(ctx.cmd, 3, 1, 0, 0);
    }
    // end dynamic rendering
    vkCmdEndRendering(ctx.cmd);
}

void MainRenderSystem::createPipeline(VkDevice device, MainPipelineContext& pipelineCtx) {
    pipelineConfig.shaderStages = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = pipelineCtx.vertexShader,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = pipelineCtx.fragmentShader,
            .pName = "main"
        }
    };
    // TODO: the color and depth format may be the same of the one in the Image class: look into it.
    pipeline.createPipeline(device, pipelineConfig);
}

void MainRenderSystem::destroyRenderSystem() {
    pipeline.destroyPipeline();
}
