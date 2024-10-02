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
    void setSurface(const VkSurfaceKHR surface);
    void createLogicalDevice();
    void destoryLogicalDevice();
    VkPhysicalDevice getCurrentPhysicalDevice() const;
    VkDevice getCurrentLogicalDevice() const;
    bool isDeviceSuitable(const VkPhysicalDevice device) const;
    SeQueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const;
    SeSwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device) const;
    bool checkDeviceExtensionSupport(const VkPhysicalDevice device) const;

  private:
    SeVulkanManager(const SeVulkanManager &) = delete;
    SeVulkanManager &operator=(const SeVulkanManager &) = delete;
    VkPhysicalDevice getBestDevice() const;

    const std::vector<const char *> m_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance m_vulkan_instance = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> m_physical_devices;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_best_physical_device = VK_NULL_HANDLE;
    VkDevice m_logical_device = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;
};

#endif