#ifndef SE_VULKAN_MANAGER_H
#define SE_VULKAN_MANAGER_H

#include <vector>
#include <vulkan/vulkan.h>

class SeVulkanManager {
  public:
    void availableExtensions();
    void availableLayers();
    void createInstance();
    void enumerateDevice();
    void createLogicDevice();
    void destoryInstance();

  private:
    VkInstance m_vulkan_instance;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkDevice m_logic_device;
};

#endif