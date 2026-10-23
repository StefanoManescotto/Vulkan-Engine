//
// Created by stefano on 28/07/26.
//

#include "render/mainRenderSystem.h"
#include "render/renderer.h"

#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

struct MeshPushConstants {
    uint64_t vertexBufferAddress; // Direct BDA address to vertices
    uint64_t _padding;
    // uint32_t textureIndex;
    glm::vec4 color;
    glm::mat4 modelMatrix;
};

struct FrameConstant { 
    uint64_t vertexBufferAddress;
    uint64_t materialBufferAddress;
    uint64_t renderItemsAddress;
};

void MainRenderSystem::init(VkDevice device, MainPipelineContext& pipelineCtx) {}

void MainRenderSystem::init2(VkDevice device, MainPipelineContext& pipelineCtx, TextureStorage& texStorage, PipelineManager& pManager) {
    PipelineManager::getDefaultConfigs(pipelineConfig);
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
        .size = sizeof(FrameConstant)
    };

    std::array<VkDescriptorSetLayout, 1> dsLayouts{ texStorage.globalDSLayout };

    // need to define a pipeline layout
    pipelineConfig.pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = dsLayouts.size(),
        .pSetLayouts = dsLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pipelineConfig.pushConstantRange
    };

    createPipeline(device, pipelineCtx, pManager);
}

void MainRenderSystem::render(MainRenderContext &ctx){}

void MainRenderSystem::render2(MainRenderContext& ctx, TextureStorage &texStorage,  PipelineManager& pManager) {
    Scene scene = *ctx.scene;
// Push root nodes to render stack
    std::vector<std::pair<Node *, glm::mat4>> m_nodeRenderStack;
    uint32_t nodeId = scene.nodeWorld.rootNodeId;
    while(nodeId) {
        Node &node = scene.nodeWorld.getNode(nodeId);
        m_nodeRenderStack.push_back({ &node, glm::mat4(1.0) });
        nodeId = node.nextSiblingId;
    }

    uint32_t drawIndex = 0;
    while(!m_nodeRenderStack.empty()) {
        auto [node, parentTransform] = m_nodeRenderStack.back();
        m_nodeRenderStack.pop_back();
        glm::mat4 matWorld = parentTransform * node->getTransform().getModelMatrix();

        if(node->meshId){
            Mesh &mesh = scene.meshContainer.meshes[node->meshId - 1];
            for(SubMesh &subMesh : mesh.subMeshes){
                ctx.indirectDrawPtr[drawIndex] = VkDrawIndexedIndirectCommand {
                    .indexCount = static_cast<uint32_t>(subMesh.indexCount),
                    .instanceCount = 1,
                    .firstIndex = static_cast<uint32_t>(subMesh.indexStart),
                    .vertexOffset = static_cast<int32_t>(subMesh.vertexStart),
                    .firstInstance = drawIndex
                };

                ctx.renderItemPtr[drawIndex] = RenderItem {
                    .wvp = scene.camera.projection * scene.camera.view * matWorld,
                    .worldMatrix = matWorld,
                    .materialIndex = subMesh.materialId - 1
                };
                drawIndex++;
            }
        }

        uint32_t childNodeId = node->firstChildId;
		while(childNodeId) {
			Node &child = scene.nodeWorld.getNode(childNodeId);
			m_nodeRenderStack.push_back({ &child, matWorld });
			childNodeId = child.nextSiblingId;
		}
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

    vkCmdBindDescriptorSets(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pManager.getPipeline(0).pipelineLayout, 0, 1,
                                &texStorage.globalDescSet, 0, nullptr);
    

    FrameConstants frameConsts;
	GPUBuffer &vertBuffer = ctx.buffers->buffers[ctx.buffers->vertexBufferId - 1];
	GPUBuffer &materialBuffer = ctx.buffers->buffers[ctx.buffers->matBufferId - 1];
	frameConsts.vertexBufferAddress = vertBuffer.deviceAddress;
	frameConsts.materialBufferAddress = materialBuffer.deviceAddress;
	frameConsts.renderItemsAddress = ctx.renderItemBuffer->deviceAddress;
	vkCmdPushConstants(ctx.cmd, pManager.getPipeline(0).pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, sizeof(FrameConstants), &frameConsts);

	GPUBuffer &idxBuffer = ctx.buffers->buffers[ctx.buffers->indexBufferId - 1];
	vkCmdBindIndexBuffer(ctx.cmd, idxBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);


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

        vkCmdBindPipeline(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pManager.getPipeline(0).handle);

        vkCmdDrawIndexedIndirect(ctx.cmd, ctx.indirectDrawBuffer->buffer, 0, drawIndex, sizeof(VkDrawIndexedIndirectCommand));
    }
    // end dynamic rendering
    vkCmdEndRendering(ctx.cmd);
}

void MainRenderSystem::createPipeline(VkDevice device, MainPipelineContext& pipelineCtx){}
void MainRenderSystem::createPipeline(VkDevice device, MainPipelineContext& pipelineCtx, PipelineManager& pManager) {
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
    pManager.createPipeline(pipelineConfig);
    // pipeline.createPipeline(device, pipelineConfig);
}

void MainRenderSystem::destroyRenderSystem() {
    // pipeline.destroyPipeline();
}
