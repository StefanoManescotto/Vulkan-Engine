//
// Created by stefano on 17/09/26.
//

#include "materialSystem.h"
#include "render/renderer.h"
#include "scene/material.h"

namespace MaterialSystem {
    std::vector<uint32_t> loadMaterials(const tg3_model &model, const std::vector<uint32_t> &textureIds, 
        std::vector<Material> &materials, std::vector<GPUMaterial> &gpuMat, Renderer& renderer) {
            
        std::vector<uint32_t> materialIds(model.materials_count);
        for (int i = 0; i < model.materials_count; ++i) {
            const tg3_material *tg3mat = &model.materials[i];
            gpuMat.push_back( GPUMaterial {
                    .baseColor = glm::vec4(
                        tg3mat->pbr_metallic_roughness.base_color_factor[0],
                        tg3mat->pbr_metallic_roughness.base_color_factor[1],
                        tg3mat->pbr_metallic_roughness.base_color_factor[2],
                        tg3mat->pbr_metallic_roughness.base_color_factor[3]),
                    .textureIndex = tg3mat->pbr_metallic_roughness.base_color_texture.index != -1
                        ? textureIds[tg3mat->pbr_metallic_roughness.base_color_texture.index] - 1
                        : 0
                });
            materialIds[i] = gpuMat.size();
            
            PipelineKey key {
                .vertShader = renderer.getShaderManager().getShaderModule("vertexShader"),
                .fragShader = renderer.getShaderManager().getShaderModule("fragmentShader"),
                
                .colorAttachmentFormat = VK_FORMAT_B8G8R8A8_SRGB,
                .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
                
                .enableBlending = false,
                .enableDepthTest = true
            };
            uint32_t pIndex = renderer.getPipelineManager().getOrCreatePipeline(key, renderer.getFrameLayout());

            materials.push_back( Material {
                .gpuMatIndex = materialIds[i],
                .pipelineIndex = pIndex,
            });
        }
        return materialIds;
    }
}
