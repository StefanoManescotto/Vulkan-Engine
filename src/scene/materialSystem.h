//
// Created by stefano on 17/09/26.
//

#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include <tiny_gltf_v3.h>

#include "render/renderer.h"
#include "scene/material.h"
// #include "vk/pipelineManager.h"

namespace MaterialSystem {
    std::vector<uint32_t> loadMaterials(const tg3_model &model, const std::vector<uint32_t> &textureIds, 
        std::vector<Material> &materials, std::vector<GPUMaterial> &gpuMat, Renderer& renderer);
}
