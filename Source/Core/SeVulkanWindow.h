#ifndef SE_VULKAN_WINDOW_H
#define SE_VULKAN_WINDOW_H
#include "SeVulkanManager.h"
#include <QScopedPointer>
#include <QWindow>

class SeVulkanWindowPrivate;
class SeVulkanWindow : public QWindow {
    Q_OBJECT
  public:
    SeVulkanWindow(SeVulkanManager *vulkan_manager);
    SeVulkanWindow(QWindow *parent, SeVulkanManager *vulkan_manager);
    ~SeVulkanWindow();
    void init();
    void cleanup();

  private:
    void createSurface();
    void destroySurface();

    void createLogicalDevice();
    void destoryLogicalDevice();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    void createSwapChain();
    void destroySwapChain();

    void createImageViews();
    void destoryImageViews();

    const std::vector<const char *> m_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    SeVulkanManager *m_vulkan_manager = nullptr;

    VkPhysicalDevice m_best_physical_device = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkDevice m_logical_device = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;

    VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swap_chain_images;
    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;

    std::vector<VkImageView> m_swap_chain_image_views;
};

#endif