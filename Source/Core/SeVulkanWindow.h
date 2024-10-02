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
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    void createSwapChain();
    void destroySwapChain();

    SeVulkanManager *m_vulkan_manager = nullptr;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swap_chain_images;
    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;
};

#endif