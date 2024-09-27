#include "Core/SeVulkanManager.h"
#include "Core/SeVulkanWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    SeVulkanManager::printAvailableExtensions();
    SeVulkanManager::printAvailableLayers();
    SeVulkanManager vulkan_manager;
    vulkan_manager.init();

    SeVulkanWindow vulkan_window(nullptr, &vulkan_manager);
    vulkan_window.show();

    return app.exec();
}
