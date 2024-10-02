#include "SeVulkanWindow.h"
#include <QDebug>
#include <algorithm>

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
    m_vulkan_manager->setSurface(m_surface);
    m_vulkan_manager->createLogicalDevice();
    createSwapChain();
}

void SeVulkanWindow::cleanup() {
    destroySwapChain();
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
    SeSwapChainSupportDetails details = m_vulkan_manager->querySwapChainSupport(m_vulkan_manager->getCurrentPhysicalDevice());

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

    SeQueueFamilyIndices indices = m_vulkan_manager->findQueueFamilies(m_vulkan_manager->getCurrentPhysicalDevice());
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
    result = vkCreateSwapchainKHR(m_vulkan_manager->getCurrentLogicalDevice(), &create_info, nullptr, &m_swap_chain);
    if (result == VK_SUCCESS) {
        qDebug() << "Swap chain created";
    } else {
        qDebug() << "Failed to create swap chain!";
    }
    assert(result == VK_SUCCESS);

    vkGetSwapchainImagesKHR(m_vulkan_manager->getCurrentLogicalDevice(), m_swap_chain, &image_count, nullptr);
    m_swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_vulkan_manager->getCurrentLogicalDevice(), m_swap_chain, &image_count, m_swap_chain_images.data());
    m_swap_chain_image_format = surface_format.format;
    m_swap_chain_extent = extent;
}

void SeVulkanWindow::destroySwapChain() {
    if (m_swap_chain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_vulkan_manager->getCurrentLogicalDevice(), m_swap_chain, nullptr);
        m_swap_chain = VK_NULL_HANDLE;
        qDebug() << "Swap chain destroyed";
    }
}

#pragma endregion Swap chain