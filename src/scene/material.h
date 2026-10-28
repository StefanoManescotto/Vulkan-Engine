//
// Created by stefano on 09/08/26.
//

#pragma once

#include <glm/vec4.hpp>

struct GPUMaterial {
    glm::vec4 baseColor = glm::vec4(1, 1, 1, 1);
    uint32_t textureIndex = 0;
};

struct Material {
    uint32_t gpuMatIndex = 0;
    uint32_t pipelineIndex;
};
