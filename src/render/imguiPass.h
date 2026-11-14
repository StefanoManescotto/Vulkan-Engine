#pragma once

#include "scene/imGuiManager.h"
#include "vk/GPUImage.h"
#include <volk.h>
#include <vulkan/vulkan_core.h>

struct GuiPassCtx {
    VkCommandBuffer cmd;
    VkExtent2D extent;
    GPUImage* colorImage;
    ImGuiManager* guiManager;
    Scene* scene;
    uint32_t nDraws;
};

class ImGuiPass {
public:
    void render(GuiPassCtx ctx);
};
