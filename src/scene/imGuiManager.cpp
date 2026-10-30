#include "imGuiManager.h"
#include "utils.h"
#include "vk/swapchain.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>
#include <fmt/printf.h>

void ImGuiManager::init(SDL_Window* window, VulkanCore& vkCore){
    m_device = vkCore.device.handle();
    createDescriptorPool(vkCore.device.handle());
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableSetMousePos;
    // io.DisplaySize.x = 

    ImGui::GetStyle().FontScaleMain = 1.2f;
    ImGui::StyleColorsDark();

    VkPipelineRenderingCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &vkCore.swapchain.swapchainFormat2,
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };
    ImGui_ImplSDL3_InitForVulkan(window);
    
    ImGui_ImplVulkan_InitInfo init_info = {
        .Instance = vkCore.instance,
        .PhysicalDevice = vkCore.device.getPhysicalDevice(),
        .Device = vkCore.device.handle(),
        .Queue = vkCore.device.getGraphicsQueue(),
        .DescriptorPool = m_descriptorPool,
        .MinImageCount = vkCore.swapchain.getImageCount(),
        .ImageCount = vkCore.swapchain.getImageCount(),
        .PipelineInfoMain = {
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .PipelineRenderingCreateInfo = pipelineInfo
        },
        .UseDynamicRendering = true,
        .Allocator = nullptr,
        .CheckVkResultFn = checkImGuiResult,
    };


    ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, [](const char* function_name, void* user_data) {
        auto instance = *static_cast<VkInstance*>(user_data);
        return vkGetInstanceProcAddr(instance, function_name);
    }, &vkCore.instance);
    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiManager::createDescriptorPool(VkDevice device){
    VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes),
        .pPoolSizes = poolSizes
    };
    VK_CHECK( vkCreateDescriptorPool(device, &pool_info, nullptr, &m_descriptorPool) );
}

void ImGuiManager::checkImGuiResult(VkResult res){
    if(res == VK_SUCCESS){
        return;
    }
    
    fmt::print("[IMGUI] Error: VkResult={}", static_cast<int>(res));
    abort();
}

void ImGuiManager::beginFrame(){
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Vulkan");

    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }

    if (ImGui::CollapsingHeader("Scene Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Object Scale", &slider1, 0.005f, 0.001f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat("Translation X", &sliderX, 0.1f); // Min/max default to unbounded if omitted/zero
        ImGui::DragFloat("Translation Y", &sliderY, 0.1f);
        ImGui::DragFloat("Translation Z", &sliderZ, 0.1f);
    }

    ImGui::End();
}

void ImGuiManager::render(VkCommandBuffer cmd){
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void ImGuiManager::destroy(){
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}


