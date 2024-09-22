#include "SeVulkanManager.h"
#include <QDebug>

void SeVulkanManager::printAvailableExtensions() const {
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

void SeVulkanManager::printAvailableLayers() const {
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
    VkResult result;
    uint32_t device_count = 0;
    result = vkEnumeratePhysicalDevices(m_vulkan_instance, &device_count, nullptr);
    m_physical_devices.resize(device_count);
    if (result == VK_SUCCESS) {
        result = vkEnumeratePhysicalDevices(m_vulkan_instance, &device_count, m_physical_devices.data());
    }
    if (result == VK_SUCCESS) {
        qDebug() << "Physical devices detected: " << device_count << " in total";
    } else {
        qDebug() << "Failed to enumerate physical devices: " << result;
    }
    assert(result == VK_SUCCESS);
    for (auto itr = m_physical_devices.begin(); itr != m_physical_devices.end(); itr++) {
        printDeviceProperties(*itr);
    }
}

void SeVulkanManager::printDeviceProperties(const VkPhysicalDevice device) const {
    VkPhysicalDeviceProperties property;
    vkGetPhysicalDeviceProperties(device, &property);
    qDebug() << property.deviceName << ":\n"
             << " deviceID:"
             << property.deviceID << "\n"
             << " vendorID:"
             << property.vendorID << "\n"
             << " deviceType:"
             << property.deviceType << "\n"
             << " driverVersion:"
             << property.driverVersion << "\n";
}

VkPhysicalDevice SeVulkanManager::getBestDevice() const {
    VkPhysicalDevice best_device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties best_device_properites;
    int maxScore = 0;
    for (const auto &device : m_physical_devices) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);
        int score = 0;

        if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        } else if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 500;
        }

        score += memory_properties.memoryHeaps[0].size / (1024 * 1024);

        if (score > maxScore) {
            maxScore = score;
            best_device = device;
            best_device_properites = device_properties;
        }
    }

    if (best_device != VK_NULL_HANDLE) {
        qDebug() << "Best physical device: " << best_device_properites.deviceName;
    } else {
        qDebug() << "No suitable physical device found!";
    }
}

void SeVulkanManager::createLogicDevice() {
    VkPhysicalDevice physical_device = getBestDevice();
    assert(physical_device != VK_NULL_HANDLE);
}

void SeVulkanManager::destoryInstance() {
    vkDestroyInstance(m_vulkan_instance, nullptr);
    qDebug() << "Vulkan instance destoryed";
}