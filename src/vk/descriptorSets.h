#include "vk/GPUImage.h"
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>

struct DescriptorConfig {
    VkDescriptorType type;
    uint32_t count = 1;
    VkShaderStageFlags stageFlags;
};

class DescriptorSets {
public:
    bool createDescriptorSetsImages(const VkDevice &device, const size_t descCount, const DescriptorConfig& configs);
    bool createDescriptorSetsUniform(const VkDevice &device, const size_t descCount, const DescriptorConfig& configs);
    void updateTextureDescriptors(const uint32_t setIndex, const GPUImage* imageViews, const VkSampler* samplers, const size_t imageCount);
    void updateUniformDescriptors(const uint32_t setIndex, const VkBuffer buffer, size_t range, uint32_t offset = 0);

    void destroyDescriptors();

    VkDescriptorSet& getDescriptorSet(){ return m_descSets; }
    VkDescriptorSetLayout& getDescLayout(const uint32_t index) { return m_descLayout; }

private:
    VkDevice m_device;
    VkDescriptorPool m_descPool;
    VkDescriptorSet m_descSets;
    VkDescriptorSetLayout m_descLayout;
    std::vector<VkDescriptorType> m_descTypes;
};
