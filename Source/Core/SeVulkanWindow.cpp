#include "SeVulkanWindow.h"
#include <QDebug>
#include <algorithm>
#include <set>

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
    m_best_physical_device = m_vulkan_manager->getBestDevice(m_surface, m_device_extensions);
    createLogicalDevice();
    createSwapChain();
}

void SeVulkanWindow::cleanup() {
    destroySwapChain();
    destoryLogicalDevice();
    m_best_physical_device = VK_NULL_HANDLE;
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

#pragma region Logical device
void SeVulkanWindow::createLogicalDevice() {
    assert(m_best_physical_device != VK_NULL_HANDLE);

    SeQueueFamilyIndices queue_family_indices = m_vulkan_manager->findQueueFamilies(m_best_physical_device, m_surface);

    std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
    std::set<uint32_t> queue_families = {queue_family_indices.graphic_family.value(), queue_family_indices.present_family.value()};
    float queue_priority = 1.0f;
    for (auto queue_family : queue_families) {
        VkDeviceQueueCreateInfo device_queue_create_info{};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.queueFamilyIndex = queue_family_indices.graphic_family.value();
        device_queue_create_info.pQueuePriorities = &queue_priority;
        device_queue_create_info.pNext = nullptr;
        device_queue_create_info.flags = 0;
        device_queue_create_infos.push_back(device_queue_create_info);
    }

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_families.size());
    VkPhysicalDeviceFeatures device_features{};
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size());
    device_create_info.ppEnabledExtensionNames = m_device_extensions.data();
    device_create_info.enabledLayerCount = 0;
    VkResult result = vkCreateDevice(m_best_physical_device, &device_create_info, nullptr, &m_logical_device);
    if (result == VK_SUCCESS) {
        qDebug() << "Logical device created";
    } else {
        qDebug() << "Failed to create logical device!";
    }
    assert(result == VK_SUCCESS);

    vkGetDeviceQueue(m_logical_device, queue_family_indices.graphic_family.value(), 0, &m_graphics_queue);
    vkGetDeviceQueue(m_logical_device, queue_family_indices.present_family.value(), 0, &m_present_queue);
}

void SeVulkanWindow::destoryLogicalDevice() {
    if (m_logical_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_logical_device, nullptr);
        m_graphics_queue = VK_NULL_HANDLE;
        m_logical_device = VK_NULL_HANDLE;
        qDebug() << "Logical device destoryed";
    }
}

#pragma endregion Logical device

#pragma region Swap chain
VkSurfaceFormatKHR SeVulkanWindow::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (const auto &available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR SeVulkanWindow::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes) {
    for (const auto &available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SeVulkanWindow::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        width = QWindow::width();
        height = QWindow::height();

        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

void SeVulkanWindow::createSwapChain() {
    SeSwapChainSupportDetails details = m_vulkan_manager->querySwapChainSupport(m_best_physical_device, m_surface);

    VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(details.formats);
    VkPresentModeKHR present_mode = chooseSwapPresentMode(details.present_modes);
    VkExtent2D extent = chooseSwapExtent(details.capabilities);

    uint32_t image_count = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
        image_count = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    SeQueueFamilyIndices indices = m_vulkan_manager->findQueueFamilies(m_best_physical_device, m_surface);
    uint32_t queueFamilyIndices[] = {indices.graphic_family.value(), indices.present_family.value()};

    if (indices.graphic_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result;
    result = vkCreateSwapchainKHR(m_logical_device, &create_info, nullptr, &m_swap_chain);
    if (result == VK_SUCCESS) {
        qDebug() << "Swap chain created";
    } else {
        qDebug() << "Failed to create swap chain!";
    }
    assert(result == VK_SUCCESS);

    vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &image_count, nullptr);
    m_swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &image_count, m_swap_chain_images.data());
    m_swap_chain_image_format = surface_format.format;
    m_swap_chain_extent = extent;
}

void SeVulkanWindow::destroySwapChain() {
    if (m_swap_chain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
        m_swap_chain = VK_NULL_HANDLE;
        qDebug() << "Swap chain destroyed";
    }
}

#pragma endregion Swap chain