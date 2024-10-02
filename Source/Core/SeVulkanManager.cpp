#include "SeVulkanManager.h"
#include <QDebug>
#include <set>
#include <string>

#pragma region Print
void SeVulkanManager::printAvailableExtensions() {
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

void SeVulkanManager::printAvailableLayers() {
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

void SeVulkanManager::printDeviceProperties(const VkPhysicalDevice device) {
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

#pragma endregion Static

#pragma region Init and cleanup

SeVulkanManager::SeVulkanManager() {
}

SeVulkanManager::~SeVulkanManager() {
    cleanup();
}

void SeVulkanManager::init() {
    createInstance();
    enumerateDevice();
}

void SeVulkanManager::cleanup() {
    destoryInstance();
}

#pragma endregion Init and cleanup

#pragma region Vulkan instance
void SeVulkanManager::createInstance() {
    // TODO:select extensions by platform
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

VkInstance SeVulkanManager::getInstance() const {
    return m_vulkan_instance;
}

#pragma endregion Vulkan instance

#pragma region Physical device
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

VkPhysicalDevice SeVulkanManager::getBestDevice(const VkSurfaceKHR surface, const std::vector<const char *> &device_extensions) const {
    assert(surface != VK_NULL_HANDLE);

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

        if (score > maxScore && isDeviceSuitable(device, surface, device_extensions)) {
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
#pragma endregion Physical device

#pragma region Device verification
bool SeVulkanManager::isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface, const std::vector<const char *> &device_extensions) const {
    return findQueueFamilies(device, surface).isComplete() && checkDeviceExtensionSupport(device, device_extensions) && querySwapChainSupport(device, surface).isSwapChainAdequate();
}

#pragma endregion Device verification

#pragma region Queue family
SeQueueFamilyIndices SeVulkanManager::findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface) const {
    assert(device != VK_NULL_HANDLE && surface != VK_NULL_HANDLE);

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
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            indices.present_family = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

#pragma endregion Queue family

#pragma region Swap chain
bool SeVulkanManager::checkDeviceExtensionSupport(const VkPhysicalDevice device, const std::vector<const char *> &device_extensions) const {
    assert(device != VK_NULL_HANDLE);

    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for (const auto &extension : extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

SeSwapChainSupportDetails SeVulkanManager::querySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR surface) const {
    assert(device != VK_NULL_HANDLE && surface != VK_NULL_HANDLE);

    SeSwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &(details.capabilities));

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    uint32_t mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, nullptr);
    if (mode_count != 0) {
        details.present_modes.resize(mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, details.present_modes.data());
    }

    return details;
}

#pragma endregion Swap chain