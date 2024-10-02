#ifndef SE_VULKAN_MANAGER_H
#define SE_VULKAN_MANAGER_H
#define VK_USE_PLATFORM_WIN32_KHR

#include "SeQueueFamilyIndices.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

class SeVulkanManager {
  public:
    static void printAvailableLayers();
    static void printAvailableExtensions();

    SeVulkanManager();
    ~SeVulkanManager();
    void init();
    void cleanup();
    void createInstance();
    void destoryInstance();
    VkInstance getInstance() const;
    void enumerateDevice();
    void printDeviceProperties(const VkPhysicalDevice device) const;
    void setSurface(VkSurfaceKHR surface);
    void createLogicalDevice();
    void destoryLogicalDevice();

  private:
    SeVulkanManager(const SeVulkanManager &) = delete;
    SeVulkanManager &operator=(const SeVulkanManager &) = delete;
    bool isDeviceSuitable(const VkPhysicalDevice device) const;
    SeQueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const;
    VkPhysicalDevice getBestDevice() const;

    VkInstance m_vulkan_instance = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> m_physical_devices;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_best_physical_device = VK_NULL_HANDLE;
    VkDevice m_logical_device = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;
};

#endif