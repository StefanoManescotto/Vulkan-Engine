//
// Created by stefano on 12/09/26.
//

#pragma once
#include <filesystem>

#include "render/renderer.h"
#include "scene/scene.h"
// #include "textureStorage.h"
#include "vk/VulkanContext.h"
// #include "vk/commandContext.h"
#include "vk/gpuBuffer.h"

namespace SceneLoader {
    Scene loadScene(VulkanCtx& ctx, Renderer& renderer, std::filesystem::path path, GPUBufferVec &GPUBuffers);
}
