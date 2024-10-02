#ifndef SE_VULKAN_MANAGER_H
#define SE_VULKAN_MANAGER_H
#define VK_USE_PLATFORM_WIN32_KHR

#include "SeQueueFamilyIndices.h"
#include "SeSwapChainSupportDetails.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

class SeVulkanManager {
  public:
    static void printAvailableLayers();
    static void printAvailableExtensions();
    static void printDeviceProperties(const VkPhysicalDevice device);

    SeVulkanManager();
    ~SeVulkanManager();
    void init();
    void cleanup();

    void createInstance();
    void destoryInstance();
    VkInstance getInstance() const;

    void enumerateDevice();

    VkPhysicalDevice getBestDevice(const VkSurfaceKHR surface, const std::vector<const char *> &device_extensions) const;
    bool isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface, const std::vector<const char *> &device_extensions) const;
    SeQueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface) const;
    SeSwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR surface) const;
    bool checkDeviceExtensionSupport(const VkPhysicalDevice device, const std::vector<const char *> &device_extensions) const;

  private:
    SeVulkanManager(const SeVulkanManager &) = delete;
    SeVulkanManager &operator=(const SeVulkanManager &) = delete;

    VkInstance m_vulkan_instance = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> m_physical_devices;
};

#endif