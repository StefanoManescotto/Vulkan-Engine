#pragma once

#include <SDL3/SDL.h>
#include <imgui/imgui.h>
#include <volk.h>

#include "scene/nodeWorld.h"
#include "scene/scene.h"
#include "vulkanTypes.h"

class ImGuiManager {
public:
    void init(SDL_Window* window, VulkanCore& vkCore);
    void beginFrame(Scene& scene, uint32_t drawsCount);
    void render(VkCommandBuffer cmd);
    void destroy();

private:
    void makeNodesTree(uint32_t nodeId,  NodeWorld& nodeWorld);
    void manageInput(NodeWorld& nodeWorld);

    VkDevice m_device;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    ImGuiTreeNodeFlags treeNodeFlags = /*ImGuiTreeNodeFlags_DefaultOpen |*/ ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow 
                            | ImGuiTreeNodeFlags_SpanAvailWidth;;
    Node* selectedNode = nullptr;

    void createDescriptorPool(VkDevice device);
    static void checkImGuiResult(VkResult res);
};
