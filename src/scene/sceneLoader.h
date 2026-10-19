//
// Created by stefano on 12/09/26.
//

#pragma once
#include <filesystem>

#include "scene/scene.h"
#include "textureStorage.h"
#include "vk/VulkanContext.h"
#include "vk/commandContext.h"
#include "vk/gpuBuffer.h"

namespace SceneLoader {
    Scene loadScene(VulkanCtx ctx, CommandContext& cmdContext, TextureStorage &texStorage, std::filesystem::path path, GPUBufferVec &GPUBuffers);
}
