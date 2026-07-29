//
// Created by stefano on 25/07/26.
//

#include "device.h"

#include <stdexcept>
#include <vector>

void Device::create(VkSurfaceKHR surface) {
    m_surface = surface;
}

Device::~Device() {
    destroyDevice();
}

VkDevice Device::handle() const {
    return m_device;
}

VkPhysicalDevice Device::getPhysicalDevice() const {
    return m_physicalDevice;
}

VkQueue Device::getGraphicsQueue() const {
    return m_queue;
}

uint32_t Device::getGraphicsQueueIndex() const {
    return m_queueFamilyIndex;
}

VkPhysicalDevice Device::findPhysicalDevice(VkInstance instance) {
    // enumerate all physical devices
    uint32_t physDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physDeviceCount, physicalDevices.data());

    if (physDeviceCount) {
        m_physicalDevice = physicalDevices[0];
        for (auto &pDev: physicalDevices) {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(pDev, &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                m_physicalDevice = pDev;
                break;
            }
        }
    }

    if (!m_physicalDevice) {
        throw std::runtime_error("Unable to find an appropriate physical device");
    }

    return m_physicalDevice;
}

uint32_t Device::findGraphicsQueue() {
    uint32_t queueFamCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(m_physicalDevice, &queueFamCount, nullptr);
    std::vector<VkQueueFamilyProperties2> queueFamProps(queueFamCount, {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
    vkGetPhysicalDeviceQueueFamilyProperties2(m_physicalDevice, &queueFamCount, queueFamProps.data());

    for (int currentFamIdx = 0; currentFamIdx < queueFamProps.size(); currentFamIdx++) {
        VkBool32 hasPresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, currentFamIdx, m_surface, &hasPresentSupport);

        const auto &props = queueFamProps[currentFamIdx];
        // ensure this is a GRAPHICS queue with presentation support
        if (props.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport) {
            m_queueFamilyIndex = currentFamIdx;
            return m_queueFamilyIndex;
        }
    }
    throw std::runtime_error("Couldn't find a graphics queue with presentation support");
}

VkDevice Device::createDevice() {
    float queuePriority = 1.0f;
    std::vector<uint32_t> queueFamilies{m_queueFamilyIndex};

    VkDeviceQueueCreateInfo gfxQueueInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_queueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    VkPhysicalDeviceVulkan14Features supportedFeatures14 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = nullptr
    };
    VkPhysicalDeviceVulkan13Features supportedFeatures13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &supportedFeatures14
    };
    VkPhysicalDeviceVulkan12Features supportedFeatures12 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &supportedFeatures13
    };
    VkPhysicalDeviceFeatures2 supportedFeatures {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &supportedFeatures12
    };
    vkGetPhysicalDeviceFeatures2(m_physicalDevice, &supportedFeatures);

    // check if what we need is supported
    if (!supportedFeatures13.dynamicRendering || !supportedFeatures13.synchronization2 ||
        !supportedFeatures12.timelineSemaphore) {
        throw std::runtime_error("Physical device doesn't meet the feature requirements");
    }

    // produce a separate features struct chain for device creation
    VkPhysicalDeviceVulkan14Features features14 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = nullptr,
    };
    VkPhysicalDeviceVulkan13Features features13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features14,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };
    VkPhysicalDeviceVulkan12Features features12 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &features13,
        .timelineSemaphore = VK_TRUE
    };
    VkPhysicalDeviceFeatures2 features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features12
    };

    const std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo devCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &gfxQueueInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = nullptr // features struct chain is set in pNext
    };

    if (vkCreateDevice(m_physicalDevice, &devCreateInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Couldn't create the device");
    }

    vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &m_queue);
    if (!m_queue) {
        throw std::runtime_error("Couldn't get the graphics queue");
    }

    return m_device;
}

void Device::destroyDevice() {
    if (m_device) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = nullptr;
    }
}


// TODO: make it so it can check on multiple possible formats and uses the first one supported

/// Ensure the swapchain format is supported
bool Device::supportSwapchainFormat(VkFormat format) const{
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, surfaceFormats.data());

    bool formatSupported = false;
    for (const VkSurfaceFormatKHR &surfFormat: surfaceFormats) {
        if (surfFormat.format == format) {
            formatSupported = true;
            break;
        }
    }
    if (!formatSupported) {
        return false;
    }
    return true;
}

bool Device::supportFormat(VkFormat format, VkFormatFeatureFlags requiredFeatures) const {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &formatProperties);

    return (formatProperties.optimalTilingFeatures & requiredFeatures) == requiredFeatures;
}
