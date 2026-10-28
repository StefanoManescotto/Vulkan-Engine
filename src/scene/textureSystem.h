//
// Created by stefano on 12/09/26.
//

#pragma once
#include <tiny_gltf_v3.h>

#include "image.h"
#include "textureStorage.h"
#include "vk/VulkanContext.h"
#include "vk/commandContext.h"

namespace TextureSystem {
    std::vector<uint32_t> loadGPUImages(VulkanCtx &ctx, CommandContext& cmdContext, TextureStorage &texStorage, std::vector<Image>& images);
    std::vector<uint32_t> loadSamplers(VkDevice &device, TextureStorage &texStorage, const tg3_model &model, std::vector<Texture> &textures);
    std::vector<uint32_t> loadTextures(size_t maxTextures, const tg3_model &model, const std::vector<uint32_t> &imageIds, const std::vector<uint32_t> &samplerIds, std::vector<Texture>& textures);
    void updateTextureDescriptors(const VkDevice &device, TextureStorage &texStorage, std::vector<Texture> &textures, VkDescriptorSet &descSet);
    bool createDescriptorSets(const VkDevice &device, TextureStorage &texStorage);
}
