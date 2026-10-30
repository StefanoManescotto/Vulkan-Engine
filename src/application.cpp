#include "application.h"
#include "render/renderer.h"
#include "scene/sceneLoader.h"
#include "vk/VulkanContext.h"
#include "vk/gpuBuffer.h"
#include "vulkanTypes.h"
#include <glm/detail/qualifier.hpp>
#include <imgui/imgui.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <fmt/printf.h>

void Application::manageInputs() {
    if (window.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running = false;
    }

    // Far plane
    if(window.isKeyDown(SDL_SCANCODE_UP)){
        m_scene.camera.farPlane++;
    }
    if(window.isKeyDown(SDL_SCANCODE_DOWN)){
        m_scene.camera.farPlane--;
    }

    if(!ImGui::GetIO().WantCaptureMouse){
        m_cameraController.updateRotation(window, m_scene.camera);
    }
    m_cameraController.updateMovement(window, m_scene.camera, m_deltaTime);

    float s1 = m_guiManager.getSlider1Value();
    Node& n = m_scene.nodeWorld.getNode(m_scene.nodeWorld.rootNodeId);
    n.getTransform().setScale(glm::vec3(s1, s1, s1));
    n.getTransform().setPosition(glm::vec3(m_guiManager.getSliderXValue(), m_guiManager.getSliderYValue(), m_guiManager.getSliderZValue()));
}

void Application::updateDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<float>(
        currentTime - m_lastFrameTime
    ).count();
    m_deltaTime = std::min(m_deltaTime, 0.1f);
    m_lastFrameTime = currentTime;
}

void Application::initialize() {
    window.createWindow();    
    
    initializeVulkan();

    VulkanCore vkCore = {
        .device = m_device,
        .instance = vulkanInstance,
        .swapchain = m_renderer.getSwapchain()
    };

    m_guiManager.init(window.handle(), vkCore);

    m_lastFrameTime = std::chrono::high_resolution_clock::now();
}

void Application::initializeVulkan() {
    createVulkanInstance();

    window.createSurface(vulkanInstance);

    m_device.create(window.getSurface());
    m_device.findPhysicalDevice(vulkanInstance);
    m_device.findGraphicsQueue();
    m_device.createDevice();
    initializeVMA();

    m_renderer.init(&m_device, &m_allocator, &window, 1024);
    
    VulkanCtx vkCtx = { .device = m_device.handle(), .allocator = m_allocator };
    
    m_scene = SceneLoader::loadScene(vkCtx, m_renderer, 
        std::string(SCENES_DIR) + "mario_kart/scene.gltf", gpuBuffers);
}

void Application::shutdown() {
    // wait in case resources are in use
    vkDeviceWaitIdle(m_device.handle());
    m_guiManager.destroy();
    m_renderer.destroyRenderer();

    for(GPUBuffer& g : gpuBuffers.buffers) {
        vmaDestroyBuffer(m_allocator, g.buffer, g.allocation);
    }

    if (m_allocator) {
        vmaDestroyAllocator(m_allocator);
    }

    window.destroySurface();

    m_device.destroyDevice();

    if (vulkanInstance) {
        vkDestroyInstance(vulkanInstance, nullptr);
    }
    volkFinalize();

    window.destroyWindow();
}

void Application::run() {
    running = true;

    while (running) {
        m_guiManager.beginFrame();
        if (!window.pollEvents()) {
            running = false;
            break;
        }

        if(window.isWindowResized()){
            m_renderer.recreateSwapchain();
        }

        updateDeltaTime();
        manageInputs();
        m_scene.camera.updateMatrices(window.getWidth(), window.getHeight());
        window.updateInputState();
        render();
    }
}

void Application::createVulkanInstance() {
    // Initialize Volk and load Vk function pointers
    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("Error initializing Volk");
    }

    // Create the vulkan application instance
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "My First Triangle",
        .apiVersion = VulkanVersion,
    };

    uint32_t instExtCount = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&instExtCount);

    std::vector requestedLayers {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = instExtCount,
        .ppEnabledExtensionNames = extensions
    };

    if (vkCreateInstance(&instCreateInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
        throw std::runtime_error("Couldn't create a vulkan instance");
    }

    volkLoadInstance(vulkanInstance);
}

void Application::initializeVMA() {
    VmaVulkanFunctions vmaFuncInfo{};
    VmaAllocatorCreateInfo vmaAllocInfo {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = m_device.getPhysicalDevice(),
        .device = m_device.handle(),
        .pVulkanFunctions = &vmaFuncInfo,
        .instance = vulkanInstance,
        .vulkanApiVersion = VulkanVersion
    };

    // vma can import directly from volk
    vmaImportVulkanFunctionsFromVolk(&vmaAllocInfo, &vmaFuncInfo);

    if (vmaCreateAllocator(&vmaAllocInfo, &m_allocator) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create Vulkan Memory Allocator");
    }
}

void Application::render() {
    m_renderer.renderFrame(&window, m_scene, gpuBuffers, m_guiManager);
}
