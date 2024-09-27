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
    SeVulkanManager *m_vulkan_manager = nullptr;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};

#endif