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

void SeVulkanManager::destoryInstance() {
    if (m_vulkan_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_vulkan_instance, nullptr);
        m_vulkan_instance = VK_NULL_HANDLE;
        qDebug() << "Vulkan instance destoryed";
    }
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

        if (score > maxScore && isDeviceSuitable(device)) {
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

    return best_device;
}

SeQueueFamilyIndices SeVulkanManager::findQueueFamilies(const VkPhysicalDevice device) const {
    SeQueueFamilyIndices indices;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
    int i = 0;
    for (const auto &queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            // VK_QUEUE_COMPUTE_BIT for compute shader and VK_QUEUE_TRANSFER_BIT for data transfer
            indices.graphic_family = i;
            if (indices.isComplete()) {
                break;
            }
        }
        i++;
    }
    return indices;
}

bool SeVulkanManager::isDeviceSuitable(const VkPhysicalDevice device) const {
    return findQueueFamilies(device).isComplete();
}

void SeVulkanManager::createLogicalDevice() {
    m_best_physical_device = getBestDevice();
    assert(m_best_physical_device != VK_NULL_HANDLE);
    SeQueueFamilyIndices queue_family_indices = findQueueFamilies(m_best_physical_device);

    VkDeviceQueueCreateInfo device_queue_create_info{};
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_create_info.queueCount = 1;
    device_queue_create_info.queueFamilyIndex = queue_family_indices.graphic_family.value();
    float queue_priority = 1.0f;
    device_queue_create_info.pQueuePriorities = &queue_priority;
    device_queue_create_info.pNext = nullptr;
    device_queue_create_info.flags = 0;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &device_queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    VkPhysicalDeviceFeatures device_features{};
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.enabledLayerCount = 0;
    VkResult result = vkCreateDevice(m_best_physical_device, &device_create_info, nullptr, &m_logical_device);
    if (result == VK_SUCCESS) {
        qDebug() << "Logical device created";
    } else {
        qDebug() << "Failed to create logical device!";
    }
    assert(result == VK_SUCCESS);
}

void SeVulkanManager::destoryLogicalDevice() {
    if (m_logical_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_logical_device, nullptr);
        m_logical_device = VK_NULL_HANDLE;
        qDebug() << "Logic device destoryed";
    }
}