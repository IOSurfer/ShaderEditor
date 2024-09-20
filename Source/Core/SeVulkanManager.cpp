#include "SeVulkanManager.h"
#include <QDebug>

void SeVulkanManager::availableExtensions() {
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    qDebug() << "Available extensions:";
    for (const auto &extension : extensions) {
        qDebug() << extension.extensionName;
    }
    qDebug() << "\n";
}

void SeVulkanManager::availableLayers() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    qDebug() << "Available layers:";
    for (const auto &layer : layers) {
        qDebug() << layer.layerName;
    }
    qDebug() << "\n";
}

void SeVulkanManager::createInstance() {
    const char *extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface"};

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "SeVulkanInstance";
    application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "SeVulkanInstance";
    application_info.apiVersion = VK_API_VERSION_1_0;
    application_info.pNext = nullptr;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.flags = 0;
    create_info.enabledExtensionCount = 2;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.pNext = nullptr;

    VkResult result;
    vkCreateInstance(&create_info, nullptr, &m_vulkan_instance);
    if (result == VK_SUCCESS) {
        qDebug() << "Vulkan instance created";
    } else {
        qDebug() << "Failed to create vulkan instance: " << result;
    }
    assert(result == VK_SUCCESS);
}

void SeVulkanManager::enumerateDevice() {
}

void SeVulkanManager::createLogicDevice() {
}

void SeVulkanManager::destoryInstance() {
    vkDestroyInstance(m_vulkan_instance, nullptr);
    qDebug() << "Vulkan instance destoryed";
}