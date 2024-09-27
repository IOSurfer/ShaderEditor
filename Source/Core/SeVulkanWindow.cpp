#include "SeVulkanWindow.h"

SeVulkanWindow::SeVulkanWindow(SeVulkanManager *vulkan_manager) : m_vulkan_manager(vulkan_manager) {
    WId win_id = winId();
}

SeVulkanWindow::SeVulkanWindow(QWindow *parent, SeVulkanManager *vulkan_manager) : QWindow(parent), m_vulkan_manager(vulkan_manager) {
    WId win_id = winId();
}

SeVulkanWindow::~SeVulkanWindow() {
}