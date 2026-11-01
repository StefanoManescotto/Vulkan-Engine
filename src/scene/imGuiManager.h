#pragma once

#include <SDL3/SDL.h>
#include <volk.h>

#include "vulkanTypes.h"

class ImGuiManager {
public:
    void init(SDL_Window* window, VulkanCore& vkCore);
    void beginFrame(uint32_t drawsCount);
    void render(VkCommandBuffer cmd);
    void destroy();

    float getSlider1Value(){ return slider1; }
    float getSliderXValue(){ return sliderX; }
    float getSliderYValue(){ return sliderY; }
    float getSliderZValue(){ return sliderZ; }

private:
    VkDevice m_device;
    float slider1 = 1.0f;
    float sliderX = .0f;
    float sliderY = -.50f;
    float sliderZ = .0f;

    void createDescriptorPool(VkDevice device);
    static void checkImGuiResult(VkResult res);

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
