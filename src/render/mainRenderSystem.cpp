//
// Created by stefano on 28/07/26.
//

#include "render/mainRenderSystem.h"
#include "render/renderer.h"

#include <algorithm>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

struct MeshPushConstants {
    uint64_t vertexBufferAddress; // Direct BDA address to vertices
    uint64_t _padding;
    glm::vec4 color;
    glm::mat4 modelMatrix;
};

struct FrameConstant { 
    uint64_t vertexBufferAddress;
    uint64_t materialBufferAddress;
    uint64_t renderItemsAddress;
};

void MainRenderSystem::render(MainRenderContext &ctx){}

void MainRenderSystem::render2(MainRenderContext& ctx, TextureStorage &texStorage,  PipelineManager& pManager) {
    std::vector<DrawItem> drawItems;
    Scene scene = *ctx.scene;
// Push root nodes to render stack
    std::vector<std::pair<Node *, glm::mat4>> m_nodeRenderStack;
    uint32_t nodeId = scene.nodeWorld.rootNodeId;
    while(nodeId) {
        Node &node = scene.nodeWorld.getNode(nodeId);
        m_nodeRenderStack.push_back({ &node, glm::mat4(1.0) });
        nodeId = node.nextSiblingId;
    }
    
    // uint32_t drawIndex = 0;
    while(!m_nodeRenderStack.empty()) {
        auto [node, parentTransform] = m_nodeRenderStack.back();
        m_nodeRenderStack.pop_back();
        glm::mat4 matWorld = parentTransform * node->getTransform().getModelMatrix();

        if(node->meshId){
            Mesh &mesh = scene.meshContainer.meshes[node->meshId - 1];
            for(SubMesh &subMesh : mesh.subMeshes) {
                drawItems.push_back( DrawItem {
                    .pipelineId = ctx.scene->materials[subMesh.materialId - 1].pipelineIndex,
                    .materialIndex = subMesh.materialId - 1,
                    .indexCount = static_cast<uint32_t>(subMesh.indexCount),
                    .firstIndex = static_cast<uint32_t>(subMesh.indexStart),
                    .vertexOffset = static_cast<int32_t>(subMesh.vertexStart),
                    .wvp = scene.camera.projection * scene.camera.view * matWorld,
                    .worldMatrix = matWorld
                });
            }
        }

        uint32_t childNodeId = node->firstChildId;
		while(childNodeId) {
			Node &child = scene.nodeWorld.getNode(childNodeId);
			m_nodeRenderStack.push_back({ &child, matWorld });
			childNodeId = child.nextSiblingId;
		}
    }

    std::sort(drawItems.begin(), drawItems.end(), [](const DrawItem& a, const DrawItem& b) {
        return a.pipelineId < b.pipelineId;
    });

    uint32_t drawIndex = 0;
    for (const DrawItem& d : drawItems) {
        ctx.indirectDrawPtr[drawIndex] = VkDrawIndexedIndirectCommand{
            .indexCount    = d.indexCount,
            .instanceCount = 1,
            .firstIndex    = d.firstIndex,
            .vertexOffset  = d.vertexOffset,
            .firstInstance = drawIndex
        };
        ctx.renderItemPtr[drawIndex] = RenderItem{
            .wvp           = d.wvp,
            .worldMatrix   = d.worldMatrix,
            .materialIndex = d.materialIndex
        };
        drawIndex++;
    }

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
            .image = ctx.depthImage.image,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
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
    VkRenderingAttachmentInfo depthAttachInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx.depthImage.imageView,
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



        int i = 0;
        uint32_t currentPipeline = -1;
        while(i < drawItems.size()){
            if(drawItems.at(i).pipelineId != currentPipeline) {
                vkCmdBindPipeline(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pManager.getPipeline(drawItems.at(i).pipelineId).handle);
                currentPipeline = drawItems.at(i).pipelineId;
            }

            uint32_t drawStart = i;
            while(i < drawItems.size() && drawItems.at(i).pipelineId == currentPipeline) i++;
            uint32_t drawCount = i - drawStart;

            vkCmdBindDescriptorSets(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pManager.getPipeline(drawItems.at(i-1).pipelineId).pipelineLayout, 0, 1,
                                        &texStorage.globalDescSet, 0, nullptr);
            
            FrameConstants frameConsts;
            GPUBuffer &vertBuffer = ctx.buffers->buffers[ctx.buffers->vertexBufferId - 1];
            GPUBuffer &materialBuffer = ctx.buffers->buffers[ctx.buffers->matBufferId - 1];
            frameConsts.vertexBufferAddress = vertBuffer.deviceAddress;
            frameConsts.materialBufferAddress = materialBuffer.deviceAddress;
            frameConsts.renderItemsAddress = ctx.renderItemBuffer->deviceAddress;

            vkCmdPushConstants(ctx.cmd, pManager.getPipeline(drawItems.at(i-1).pipelineId).pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                0, sizeof(FrameConstants), &frameConsts);

            GPUBuffer &idxBuffer = ctx.buffers->buffers[ctx.buffers->indexBufferId - 1];
            vkCmdBindIndexBuffer(ctx.cmd, idxBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);


            vkCmdDrawIndexedIndirect(ctx.cmd, ctx.indirectDrawBuffer->buffer, drawStart * sizeof(VkDrawIndexedIndirectCommand), drawCount, sizeof(VkDrawIndexedIndirectCommand));
        }
    }
    // end dynamic rendering
    vkCmdEndRendering(ctx.cmd);
}

void MainRenderSystem::destroyRenderSystem() {
    // pipeline.destroyPipeline();
}
