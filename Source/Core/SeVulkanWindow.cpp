#include "SeVulkanWindow.h"
#include <QDebug>

#pragma region Init and cleanup
SeVulkanWindow::SeVulkanWindow(SeVulkanManager *vulkan_manager) : m_vulkan_manager(vulkan_manager) {
    init();
}

SeVulkanWindow::SeVulkanWindow(QWindow *parent, SeVulkanManager *vulkan_manager) : QWindow(parent), m_vulkan_manager(vulkan_manager) {
    init();
}

SeVulkanWindow::~SeVulkanWindow() {
    cleanup();
}

void SeVulkanWindow::init() {
    createSurface();
}

void SeVulkanWindow::cleanup() {
    destroySurface();
}
#pragma endregion Init and cleanup

#pragma region Window surface
void SeVulkanWindow::createSurface() {
    // TODO: support other platform
    VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = reinterpret_cast<HWND>(winId());
    create_info.hinstance = GetModuleHandle(nullptr);
    VkResult result;
    result = vkCreateWin32SurfaceKHR(m_vulkan_manager->getInstance(), &create_info, nullptr, &m_surface);
    if (result == VK_SUCCESS) {
        qDebug() << "Surface created";
    } else {
        qDebug() << "Failed to create surface!";
    }
    assert(result == VK_SUCCESS);
}

void SeVulkanWindow::destroySurface() {
    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_vulkan_manager->getInstance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
        qDebug() << "Surface destroyed";
    }
}
#pragma endregion Window surface