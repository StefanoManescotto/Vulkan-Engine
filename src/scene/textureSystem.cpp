//
// Created by stefano on 12/09/26.
//

#include "textureSystem.h"

#include <cstring>

// #include <string>
#include <fmt/printf.h>
#include <stb_image.h>
#include <tiny_gltf_v3.h>
#include <vulkan/vulkan_core.h>

#include "vk/commandContext.h"
#include "textureStorage.h"
#include "vk/VulkanContext.h"
#include "vk/GPUImageSystem.h"
#include "vk/gpuBuffer.h"
#include "utils.h"

namespace TextureSystem {
    uint32_t createImage(const VulkanCtx &ctx, TextureStorage &texStorage, VkCommandBuffer cmd, const Image &img, GPUImage &gpuImage, GPUBuffer gpuBuffer) {
        GPUImageConfig imageConfig {
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .extent = {
                .width = static_cast<uint32_t>(img.width),
                .height = static_cast<uint32_t>(img.height)
            },
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        };
        GPUImage newImage;
        GPUImageSystem::allocateImage(ctx, imageConfig, newImage);
        texStorage.gpuImages.push_back(newImage);
        uint32_t imgId = texStorage.gpuImages.size() - 1;

        // Texture* tex = &m_textures.emplace(name, Texture()).first->second;
        // tex->m_image.allocateImage(ctx.device, ctx.allocator, imageConfig);


        GPUImageSystem::transitionLayout(texStorage.gpuImages.at(imgId), cmd, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_COPY_BIT,VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {
                .width = static_cast<uint32_t>(img.width),
                .height = static_cast<uint32_t>(img.height),
                .depth = 1
            }
        };
        vkCmdCopyBufferToImage(cmd, gpuBuffer.buffer, texStorage.gpuImages.at(imgId).image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        GPUImageSystem::transitionLayout(texStorage.gpuImages.at(imgId), cmd, VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        // });
        return imgId + 1;
    }

    std::vector<uint32_t> loadGPUImages(VulkanCtx &ctx, CommandContext& cmdContext, TextureStorage &texStorage, std::vector<Image>& images) {

        std::vector<uint32_t> imgIds;
        std::vector<VkBuffer> stagingBuffers;
        cmdContext.immediateSubmit([&](VkCommandBuffer cmd) {
            for (int i = 0; i < images.size(); i++) {
                Image img = images[i];
                VkDeviceSize imageSize = img.width * img.height * img.channels;

                GPUBuffer gpuBuffer {};
                // VkBuffer stagingBuffer;
                // VmaAllocation stagingBufferMemory;
                gpuBuffer = gpuBuffer::createBuffer(
                    ctx,
                    imageSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                    true
                );
                stagingBuffers.push_back(gpuBuffer.buffer);

                void *data = nullptr;
                vmaMapMemory(ctx.allocator, gpuBuffer.allocation, &data);
                memcpy(data, img.data, imageSize);
                vmaUnmapMemory(ctx.allocator, gpuBuffer.allocation);

                GPUImage gpuImage {};
                uint32_t newId = createImage(ctx, texStorage, cmd, img, gpuImage, gpuBuffer);
                imgIds.push_back(newId);
            }
        });

        for (VkBuffer buffer : stagingBuffers) {
            vkDestroyBuffer(ctx.device, buffer, nullptr);
        }

        return imgIds;
    }

    std::vector<uint32_t> loadSamplers(VkDevice &device, TextureStorage &texStorage, const tg3_model &model, std::vector<Texture> &textures) {
		std::vector<uint32_t> samplerIds(model.samplers_count);
		for (int i = 0; i < model.samplers_count; i++) {
			const tg3_sampler &tg3Sampler = model.samplers[i];
			static const std::unordered_map<int32_t, std::tuple<VkFilter, VkSamplerMipmapMode, float>> filterMap {
				{ TG3_TEXTURE_FILTER_NEAREST, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.25f } },
				{ TG3_TEXTURE_FILTER_LINEAR, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.25f } },
				{ TG3_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_LOD_CLAMP_NONE } },
				{ TG3_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_LOD_CLAMP_NONE } },
				{ TG3_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_LOD_CLAMP_NONE } },
				{ TG3_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_LOD_CLAMP_NONE } }
			};
			static const std::unordered_map<int32_t, VkSamplerAddressMode> wrapMap {
				{ TG3_TEXTURE_WRAP_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT },
				{ TG3_TEXTURE_WRAP_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE },
				{ TG3_TEXTURE_WRAP_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT }
			};

			VkSamplerCreateInfo samplerInfo {
				.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.magFilter = (tg3Sampler.mag_filter == -1) ? VK_FILTER_LINEAR : std::get<0>(filterMap.at(tg3Sampler.mag_filter)),
				.minFilter = (tg3Sampler.min_filter == -1) ? VK_FILTER_LINEAR : std::get<0>(filterMap.at(tg3Sampler.min_filter)),
				.mipmapMode = (tg3Sampler.min_filter == -1) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : std::get<1>(filterMap.at(tg3Sampler.min_filter)),
				.addressModeU = (tg3Sampler.wrap_s == -1) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : wrapMap.at(tg3Sampler.wrap_s),
				.addressModeV = (tg3Sampler.wrap_t == -1) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : wrapMap.at(tg3Sampler.wrap_t),
				.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.compareEnable = VK_FALSE,
				.minLod = 0.0f,
				.maxLod = (tg3Sampler.min_filter == -1) ? VK_LOD_CLAMP_NONE : std::get<2>(filterMap.at(tg3Sampler.min_filter))
			};

			VkSampler sampler = nullptr;
			if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
				fmt::print("Unable to create texture sampler");
				samplerIds[i] = textures[0].samplerId; // fallback texture's sampler
			}
			else {
				texStorage.samplers.push_back(sampler);
				samplerIds[i] = texStorage.samplers.size();
			}
		}
		return samplerIds;
	}

	std::vector<uint32_t> loadTextures(const size_t maxTextures, const tg3_model &model, const std::vector<uint32_t> &imageIds, const std::vector<uint32_t> &samplerIds, std::vector<Texture>& textures) {
    	// TODO: add assert
        assert(textures.size() + model.textures_count <= maxTextures && "Exceeding max texture count");
        std::vector<uint32_t> textureIds(model.textures_count);
        for (int i = 0; i < model.textures_count; i++) {
            const tg3_texture &tex = model.textures[i];
            textures.push_back(
                Texture {
                    .imageId = imageIds[tex.source],
                    .samplerId = samplerIds[tex.sampler]
                });
            textureIds[i] = textures.size();
        }
        return textureIds;
    }

    void updateTextureDescriptors(const VkDevice &device, TextureStorage &texStorage, std::vector<Texture> &textures, VkDescriptorSet &descSet) {
        std::vector<VkDescriptorImageInfo> imageDescriptors;
        imageDescriptors.reserve(textures.size());

        for (const Texture &texture : textures) {
            imageDescriptors.push_back({
                .sampler = texStorage.samplers[texture.samplerId - 1],
                .imageView = texStorage.gpuImages[texture.imageId - 1].imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 
            });
        }

        VkWriteDescriptorSet descSetWrite { 
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = static_cast<uint32_t>(imageDescriptors.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = imageDescriptors.data() 
        };
        vkUpdateDescriptorSets(device, 1, &descSetWrite, 0, nullptr);
    }

    bool createDescriptorSets(const VkDevice &device, TextureStorage &texStorage) {
        std::array<VkDescriptorPoolSize, 1> poolSizes {
            VkDescriptorPoolSize {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(texStorage.maxTextures) 
            }
        };
        
        VkDescriptorPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = poolSizes.size(),
            .pPoolSizes = poolSizes.data()
        };

        VK_CHECK( vkCreateDescriptorPool(device, &poolInfo, nullptr, &texStorage.descPool) );

        // global descriptor set
        std::array<VkDescriptorSetLayoutBinding, 1> bindings {
        VkDescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(texStorage.maxTextures),
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
            }
        };
        std::array<VkDescriptorBindingFlags, 1> flags;
        flags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = flags.size(),
            .pBindingFlags = flags.data()
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &flagsInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = bindings.size(),
            .pBindings = bindings.data()
        };
        VK_CHECK( vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &texStorage.globalDSLayout) );

        // create the actual descriptor sets
        VkDescriptorSetAllocateInfo descSetAllocInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = texStorage.descPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &texStorage.globalDSLayout,
        };
        VK_CHECK( vkAllocateDescriptorSets(device, &descSetAllocInfo, &texStorage.globalDescSet) );

        return true;
    }



    // void addSampler(VulkanCtx &ctx, TextureStorage &texStorage) {
    //     // TODO: change this so it can be chosen any sampler from the list
    //     // tex->m_sampler = &m_samplers.at(0);
    //
    //     VkSamplerCreateInfo samplerInfo{
    //         .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    //         .magFilter = VK_FILTER_NEAREST,
    //         .minFilter = VK_FILTER_NEAREST,
    //         .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    //         .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    //         .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    //         .compareEnable = VK_FALSE,
    //     };
    //     VkSampler sampler = VK_NULL_HANDLE;
    //     if (vkCreateSampler(ctx.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
    //         throw std::runtime_error("Failed to create sampler.");
    //     }
    //
    //     texStorage.samplers.push_back(sampler);
    // }
}
