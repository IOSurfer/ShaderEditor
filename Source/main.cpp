#include "Core/SeVulkanManager.h"
int main() {
    SeVulkanManager vulkan_manager;
    vulkan_manager.printAvailableExtensions();
    vulkan_manager.printAvailableLayers();
    vulkan_manager.createInstance();
    vulkan_manager.enumerateDevice();
    vulkan_manager.createLogicDevice();
    vulkan_manager.destoryInstance();
    return 0;
}
