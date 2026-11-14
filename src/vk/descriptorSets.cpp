#include <array>
#include <vulkan/vulkan_core.h>

#include "descriptorSets.h"
#include "utils.h"
#include "vk/GPUImage.h"

void DescriptorSets::updateUniformDescriptors(const uint32_t setIndex, const VkBuffer buffer, size_t range, uint32_t offset) {
    VkDescriptorBufferInfo bufferDescriptor {
        .buffer = buffer,
        .offset = offset,
        .range = range
    };

    VkWriteDescriptorSet descSetWrite { 
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descSets,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferDescriptor 
    };
    vkUpdateDescriptorSets(m_device, 1, &descSetWrite, 0, nullptr);
}

void DescriptorSets::updateTextureDescriptors(const uint32_t setIndex, const GPUImage* gpuImages, const VkSampler* samplers, const size_t imageCount) {
    std::vector<VkDescriptorImageInfo> imageDescriptors;
    imageDescriptors.reserve(imageCount);

    for (int i = 0; i < imageCount; i++) {
        imageDescriptors.push_back({
            .sampler = samplers[i],
            .imageView = gpuImages[i].imageView,
            .imageLayout = gpuImages[i].currentLayout 
        });
    }

    VkWriteDescriptorSet descSetWrite { 
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descSets,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = static_cast<uint32_t>(imageDescriptors.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = imageDescriptors.data() 
    };
    vkUpdateDescriptorSets(m_device, 1, &descSetWrite, 0, nullptr);
}

/*
    A descriptor pool says how many and of which type descriptor sets are allowed to be created using that pool. 
    Than each descriptor set have a layout that says which type the descriptor set is and how much data is going to get (descriptor count) 
    and the binding of the descriptor set. 
    Than the descriptor pool and descriptor set layout are used to create the actual descriptor set
*/
bool DescriptorSets::createDescriptorSetsImages(const VkDevice& device, const size_t descCount, const DescriptorConfig& configs) {
    m_device = device;

    // descriptor set pool
    std::array<VkDescriptorPoolSize, 1> poolSizes {
        VkDescriptorPoolSize {
            .type = configs.type,
            .descriptorCount = configs.count
        }
    };
    
    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    VK_CHECK( vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descPool) );

    // descriptor sets binding and layout
    std::array<VkDescriptorSetLayoutBinding, 1> bindings {
    VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = configs.type,
            .descriptorCount = configs.count,
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
    VK_CHECK( vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descLayout) );

    // create the descriptor sets
    VkDescriptorSetAllocateInfo descSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descLayout,
    };
    VK_CHECK( vkAllocateDescriptorSets(device, &descSetAllocInfo, &m_descSets) );
    
    return true;
}

bool DescriptorSets::createDescriptorSetsUniform(const VkDevice& device, const size_t descCount, const DescriptorConfig& configs) {
    m_device = device;

    // descriptor set pool
    std::array<VkDescriptorPoolSize, 1> poolSizes {
        VkDescriptorPoolSize {
            .type = configs.type,
            .descriptorCount = configs.count
        }
    };
    
    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    VK_CHECK( vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descPool) );

    // descriptor sets binding and layout
    std::array<VkDescriptorSetLayoutBinding, 1> bindings {
    VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = configs.type,
            .descriptorCount = configs.count,
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
    VK_CHECK( vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descLayout) );

    // create the descriptor sets
    VkDescriptorSetAllocateInfo descSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descLayout,
    };
    VK_CHECK( vkAllocateDescriptorSets(device, &descSetAllocInfo, &m_descSets) );
    
    return true;
}

void DescriptorSets::destroyDescriptors(){
    vkDestroyDescriptorPool(m_device, m_descPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descLayout, nullptr);
}
