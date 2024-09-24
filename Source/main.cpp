#include "Core/SeVulkanManager.h"
int main() {
    SeVulkanManager vulkan_manager;
    vulkan_manager.printAvailableExtensions();
    vulkan_manager.printAvailableLayers();
    vulkan_manager.createInstance();
    vulkan_manager.enumerateDevice();
    vulkan_manager.createLogicalDevice();
    vulkan_manager.destoryLogicalDevice();
    vulkan_manager.destoryInstance();
    return 0;
}
