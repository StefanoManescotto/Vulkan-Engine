//
// Created by stefano on 12/09/26.
//

#include "sceneLoader.h"

#include "render/renderer.h"
#include "scene/material.h"
#include "tiny_gltf_v3.h"
#include "stb_image.h"

#include <filesystem>
#include <stdexcept>
#include <vector>
#include <fmt/printf.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>

#include "image.h"
#include "scene/textureStorage.h"
#include "scene/textureSystem.h"
#include "scene/materialSystem.h"
#include "MeshContainer.h"
#include "scene/scene.h"
#include "scene/nodeWorld.h"
#include "scene/meshSystem.h"
#include "vk/GPUImage.h"
#include "vk/gpuBuffer.h"
// #include "scene/material.h"

namespace SceneLoader {
    void initData(VulkanCtx &ctx, CommandContext &cmd, Scene &sceneObjs, TextureStorage &texStorage, size_t vertexBufferBytes, size_t indexBufferBytes) {
        size_t totalVerts = vertexBufferBytes / sizeof(Vertex);
        size_t totalIndices = indexBufferBytes / sizeof(uint32_t);
        sceneObjs.meshContainer.vertices.resize(totalVerts);
        sceneObjs.meshContainer.indices.resize(totalIndices);

        sceneObjs.nodeWorld.initialize(1024);

        // fallback 1x1 white texture base color texture (0-index in tex array)
        uint32_t whitePixelData = 0xFFFFFFFF; // RGBA
        Image whitePixel {
            .width = 1,
            .height = 1,
            .channels = 4,
            .data = reinterpret_cast<unsigned char *>(&whitePixelData),
        };

        std::vector<Image> whitePixelVec;
        whitePixelVec.push_back(whitePixel);
        uint32_t whiteImgId = TextureSystem::loadGPUImages(ctx, cmd, texStorage, whitePixelVec).at(0);

        // fallback texture sampler
        VkSamplerCreateInfo samplerInfo {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .compareEnable = VK_FALSE
        };
        VkSampler sampler = nullptr;
        if (vkCreateSampler(ctx.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create texture smapler");
        }

        texStorage.samplers.push_back(sampler);
        uint32_t whiteSamplerId = texStorage.samplers.size();

        sceneObjs.textures.push_back(Texture{
            .imageId = whiteImgId,
            .samplerId = whiteSamplerId
        });
    }

    std::vector<Image> loadImages(tg3_model model, std::filesystem::path path) {
        std::vector<Image> images(model.images_count);
        for (int i = 0; i < model.images_count; i++) {
            Image &img = images[i];
            std::filesystem::path imagePath = path / model.images[i].uri.data; // get the absolute path
            
            fmt::print("Loading image {}/{}: {}\n", i+1, model.images_count, model.images[i].uri.data);
            
            img.data = stbi_load(imagePath.c_str(), &img.width, &img.height, &img.channels, STBI_rgb_alpha);
            if(!img.data) {
                fmt::print("Failed to load image {}\n", imagePath.c_str());
            } else { // TODO: validation error without this.
                img.channels = 4;
            }
        }
        return images;
    }

    std::vector<uint32_t> uploadImages(std::vector<Image> images) {
        std::vector<uint32_t> r;
        return r;
    }

    uint32_t importNode(NodeWorld &nodeWorld, const tg3_model &model, int32_t nodeIndex, uint32_t parentId, uint32_t prevSiblingId, std::vector<uint32_t> &meshIds) {
        const tg3_node &tg3Node = model.nodes[nodeIndex];

        auto [node, nodeId] = nodeWorld.createNode();
        node.parentId = parentId;

        if (tg3Node.has_matrix) {
            glm::mat4 transform(1);
            float *transformPtr = glm::value_ptr(transform);
            for (int i = 0; i < 16; ++i) {
                transformPtr[i] = static_cast<float>(tg3Node.matrix[i]);
            }
            node.getTransform().setTransform(transform);
        }
        else {
            glm::vec3 translation(tg3Node.translation[0], tg3Node.translation[1], tg3Node.translation[2]);
            glm::quat rotation(tg3Node.rotation[3], tg3Node.rotation[0], tg3Node.rotation[1], tg3Node.rotation[2]);
            glm::vec3 scale(tg3Node.scale[0], tg3Node.scale[1], tg3Node.scale[2]);

            node.getTransform().setPosition(translation);
            node.getTransform().setRotation(rotation);
            node.getTransform().setScale(scale);
        }

        if (tg3Node.mesh != -1) {
            node.meshId = meshIds[tg3Node.mesh];
        }

        if (prevSiblingId) {
            nodeWorld.getNode(prevSiblingId).nextSiblingId = nodeId;
        }

        // iterate through child nodes
        uint32_t lastChildId = 0;
        for (int i = 0; i < tg3Node.children_count; ++i) {
            int32_t childIndex = tg3Node.children[i];
            lastChildId = importNode(nodeWorld, model, childIndex, nodeId, lastChildId, meshIds);

            // set parent's first child id field
            if (!node.firstChildId) {
                node.firstChildId = lastChildId;
            }
        }

        return nodeId;
    }

    void loadGltf(VulkanCtx ctx, Renderer& renderer, const std::string &filepath, Scene &sceneObjs) {
        if (!std::filesystem::exists(filepath)) {
            fmt::print("File doesn't exist : {}\n", filepath);
            return;
        }
        fmt::print("Loading GLTF: {}\n", filepath);

        tg3_model model;
        tg3_parse_options opts;
        tg3_error_stack errors;

        tg3_parse_options_init(&opts);
        tg3_error_stack_init(&errors);
        tg3_error_code parseResult = tg3_parse_file(&model, &errors, filepath.c_str(), filepath.size(), &opts);
        
        if (parseResult != TG3_OK) {
            fmt::print("Parsing errors:\n");
            for (int i = 0; errors.count; i++) {
                fmt::print("{}\n", errors.entries[i].message);
            }
            tg3_error_stack_free(&errors);
            return;
        }
        tg3_error_stack_free(&errors);

        std::filesystem::path imageDir = std::filesystem::path(filepath).parent_path();
        std::vector<Image> images = loadImages(model, imageDir);
        // std::vector<uint32_t> imageIds = uploadImages(images); // upload images to VRAM
        std::vector<uint32_t> imageIds = TextureSystem::loadGPUImages(ctx, renderer.getCommandContext(), renderer.getTextureStorage(), images);
        for (const Image img : images) {
            stbi_image_free(img.data);
        }

        
        std::vector<uint32_t> samplerIds = TextureSystem::loadSamplers(ctx.device, renderer.getTextureStorage(), model, sceneObjs.textures);
        std::vector<uint32_t> textureIds = TextureSystem::loadTextures(renderer.getTextureStorage().maxTextures, model, imageIds, samplerIds, sceneObjs.textures);
        std::vector<uint32_t> materialIds = MaterialSystem::loadMaterials(model, textureIds, sceneObjs.materials, sceneObjs.gpuMaterials, renderer);
        std::vector<uint32_t> meshIds = MeshSystem::loadMeshes(model, materialIds, sceneObjs.meshContainer);

        const tg3_scene *scene = &model.scenes[model.default_scene != -1 ? model.default_scene : 0];

        std::vector<std::pair<Node *, glm::mat4>> m_nodeRenderStack;
        // iterate over the scene's roots nodes, attach to existing root node (if present)
        for (int i = 0; i < scene->nodes_count; i++) {
            uint32_t nodeId = importNode(sceneObjs.nodeWorld, model, scene->nodes[i], 0, sceneObjs.nodeWorld.lastRootNodeId, meshIds);
            if (!sceneObjs.nodeWorld.rootNodeId) { // first root node
                sceneObjs.nodeWorld.rootNodeId = nodeId;
                sceneObjs.nodeWorld.lastRootNodeId = nodeId;
            }
            else { // subsequent root nodes, link to previous root node
                sceneObjs.nodeWorld.lastRootNodeId = nodeId;
            }
        }
        tg3_model_free(&model);
        
        fmt::print("GLTF Loading Successful\n\n");
    }

    Scene loadScene(VulkanCtx& ctx, Renderer& renderer, std::filesystem::path path, GPUBufferVec &GPUBuffers) {
        Scene scene {};
        constexpr size_t vertexBufferBytes = 64 * 1024 * 1024; // 64MB
        constexpr size_t indexBufferBytes = 32 * 1024 * 1024; // 32MB

        // VkDeviceSize vertexBufferBytes = scene.meshContainer.vertices.size() * sizeof(scene.meshContainer.vertices[0]);
        // VkDeviceSize indexBufferBytes = scene.meshContainer.indices.size() * sizeof(scene.meshContainer.indices[0]);


        initData(ctx, renderer.getCommandContext(), scene, renderer.getTextureStorage(), vertexBufferBytes, indexBufferBytes);
        loadGltf(ctx, renderer, path, scene);

        Node &root = scene.nodeWorld.getNode(scene.nodeWorld.rootNodeId);
        root.getTransform().setScale(glm::vec3(0.01, 0.01, 0.01));
        root.getTransform().setPosition(glm::vec3(0, -15, 0));

        
        // --- Load into buffers ---
        GPUBuffer vertexBuffStage = gpuBuffer::createBuffer(ctx, vertexBufferBytes, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, true);

        GPUBuffer indexBuffStage = gpuBuffer::createBuffer(ctx, indexBufferBytes, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, true);

        GPUBuffer vertexBuffer = gpuBuffer::createBuffer(ctx, vertexBufferBytes, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_AUTO, false);
        GPUBuffers.buffers.push_back(vertexBuffer);
        GPUBuffers.vertexBufferId = GPUBuffers.buffers.size();
        gpuBuffer::mapCopyBufferData(ctx.allocator, vertexBuffStage, 0, scene.meshContainer.vertices.data(), vertexBufferBytes);

        GPUBuffer indexBuffer = gpuBuffer::createBuffer(ctx, indexBufferBytes, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, false);

        GPUBuffers.buffers.push_back(indexBuffer);
        GPUBuffers.indexBufferId = GPUBuffers.buffers.size();
        gpuBuffer::mapCopyBufferData(ctx.allocator, indexBuffStage, 0, scene.meshContainer.indices.data(), indexBufferBytes);
        
        // copy staged data into VRAM
        VkCommandBuffer geoCmdBuffer = renderer.getCommandContext().startTransientCommandBuffer();
        VkBufferCopy buffCopyVerts{ .srcOffset = 0, .dstOffset = 0, .size = vertexBufferBytes };
        vkCmdCopyBuffer(geoCmdBuffer, vertexBuffStage.buffer, vertexBuffer.buffer, 1, &buffCopyVerts);
        VkBufferCopy buffCopyIndices{ .srcOffset = 0, .dstOffset = 0, .size = indexBufferBytes };
        vkCmdCopyBuffer(geoCmdBuffer, indexBuffStage.buffer, indexBuffer.buffer, 1, &buffCopyIndices);
        renderer.getCommandContext().submitTransientCommandBuffer(geoCmdBuffer);
        
        vmaDestroyBuffer(ctx.allocator, vertexBuffStage.buffer, vertexBuffStage.allocation);
        vmaDestroyBuffer(ctx.allocator, indexBuffStage.buffer, indexBuffStage.allocation);
        
        TextureSystem::updateTextureDescriptors(ctx.device, renderer.getTextureStorage(), scene.textures, renderer.getTextureStorage().globalDescSet);

        // Material buffer
        const size_t matDataBytes = scene.gpuMaterials.size() * sizeof(GPUMaterial);
        GPUBuffer matBuffer = gpuBuffer::createBuffer(ctx, matDataBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
            VMA_MEMORY_USAGE_AUTO, true);
        GPUBuffers.buffers.push_back(matBuffer);
        GPUBuffers.matBufferId = GPUBuffers.buffers.size();
        gpuBuffer::mapCopyBufferData(ctx.allocator, matBuffer, 0, scene.gpuMaterials.data(), matDataBytes);

        return scene;
    }
}
