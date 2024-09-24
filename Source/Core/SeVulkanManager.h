#ifndef SE_VULKAN_MANAGER_H
#define SE_VULKAN_MANAGER_H

#include "SeQueueFamilyIndices.h"
#include <vector>
#include <vulkan/vulkan.h>

class SeVulkanManager {
  public:
    void printAvailableExtensions() const;
    void printAvailableLayers() const;
    void createInstance();
    void enumerateDevice();
    void printDeviceProperties(const VkPhysicalDevice device) const;
    SeQueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const;
    bool isDeviceSuitable(const VkPhysicalDevice device) const;
    void createLogicDevice();
    void destoryInstance();

  private:
    VkPhysicalDevice getBestDevice() const;

    VkInstance m_vulkan_instance;
    std::vector<VkPhysicalDevice> m_physical_devices;
    VkPhysicalDevice m_best_device = VK_NULL_HANDLE;
    VkDevice m_logic_device;
};

#endif