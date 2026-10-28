//
// Created by stefano on 11/09/26.
//

#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <render/GPUMesh.h>
#include <scene/MeshContainer.h>
#include <vk/VulkanContext.h>
#include <tiny_gltf_v3.h>

namespace MeshSystem {
    std::vector<uint32_t> loadMeshes(const tg3_model &model, const std::vector<uint32_t> &materialIds, MeshContainer &meshVerts);
}
