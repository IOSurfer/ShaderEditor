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

  private:
    SeVulkanManager *m_vulkan_manager = nullptr;
};

#endif