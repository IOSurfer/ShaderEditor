#include "Core/SeVulkanManager.h"
int main() {
    SeVulkanManager vulkan_manager;
    vulkan_manager.availableExtensions();
    vulkan_manager.availableLayers();
    vulkan_manager.createInstance();
    vulkan_manager.destoryInstance();
    return 0;
}
