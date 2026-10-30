#pragma once

#include <SDL3/SDL.h>
#include <volk.h>

#include "vulkanTypes.h"

class ImGuiManager {
public:
    void init(SDL_Window* window, VulkanCore& vkCore);
    void beginFrame();
    void render(VkCommandBuffer cmd);
    void destroy();

    float getSlider1Value(){ return slider1; }
    float getSliderXValue(){ return sliderX; }
    float getSliderYValue(){ return sliderY; }
    float getSliderZValue(){ return sliderZ; }

private:
    VkDevice m_device;
    float slider1 = .15f;
    float sliderX = .0f;
    float sliderY = -.10f;
    float sliderZ = .0f;

    void createDescriptorPool(VkDevice device);
    static void checkImGuiResult(VkResult res);

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
