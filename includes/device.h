//
// Created by stefano on 25/07/26.
//
#pragma once

#include <volk.h>

class Device {
public:
    Device() = default;
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    void create(VkSurfaceKHR surface);
    VkPhysicalDevice findPhysicalDevice(VkInstance instance);
    uint32_t findGraphicsQueue();
    VkDevice createDevice();
    void destroyDevice();

    bool supportSwapchainFormat(VkFormat format) const;
    bool supportFormat(VkFormat format, VkFormatFeatureFlags requiredFeatures) const;

    [[nodiscard]] VkDevice handle() const;
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const;
    [[nodiscard]] VkQueue getGraphicsQueue() const;
    [[nodiscard]] uint32_t getGraphicsQueueIndex() const;

private:
    VkDevice m_device = nullptr;
    VkPhysicalDevice m_physicalDevice = nullptr;
    uint32_t m_queueFamilyIndex = UINT32_MAX;
    VkQueue m_queue = nullptr;

    VkSurfaceKHR m_surface = nullptr;
};
