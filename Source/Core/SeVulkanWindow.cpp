#include "SeVulkanWindow.h"
#include "Util/SeUtil.h"
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
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
}

void SeVulkanWindow::cleanup() {
    destroyGraphicsPipeline();
    destroyRenderPass();
    destoryImageViews();
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
    if (m_surface) {
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
    if (m_logical_device) {
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
    if (m_swap_chain) {
        vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
        m_swap_chain = VK_NULL_HANDLE;
        qDebug() << "Swap chain destroyed";
    }
}

#pragma endregion Swap chain

#pragma region Image views
void SeVulkanWindow::createImageViews() {
    m_swap_chain_image_views.resize(m_swap_chain_images.size());
    for (size_t i = 0; i < m_swap_chain_images.size(); i++) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = m_swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = m_swap_chain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkResult result;
        result = vkCreateImageView(m_logical_device, &create_info, nullptr, &m_swap_chain_image_views[i]);
        if (result == VK_SUCCESS) {
            qDebug() << "Image view " << i << " created";
        } else {
            qDebug() << "Failed to create image view " << i << "!";
        }
        assert(result == VK_SUCCESS);
    }
}

void SeVulkanWindow::destoryImageViews() {
    for (size_t i = 0; i < m_swap_chain_images.size(); i++) {
        vkDestroyImageView(m_logical_device, m_swap_chain_image_views[i], nullptr);
        qDebug() << "Image view " << i << " destroyed";
    }
}

#pragma endregion Image views

#pragma region Render pass
void SeVulkanWindow::createRenderPass() {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = m_swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    VkResult result;
    result = vkCreateRenderPass(m_logical_device, &render_pass_info, nullptr, &m_render_pass);
    if (result == VK_SUCCESS) {
        qDebug() << "Render pass created";
    } else {
        qDebug() << "Failed to create render pass!";
    }
    assert(result == VK_SUCCESS);
}

void SeVulkanWindow::destroyRenderPass() {
    if (m_render_pass) {
        vkDestroyRenderPass(m_logical_device, m_render_pass, nullptr);
        m_render_pass = VK_NULL_HANDLE;
        qDebug() << "Render pass destroyed";
    }
}

#pragma endregion Render pass

#pragma region Graphics pipeline
void SeVulkanWindow::createGraphicsPipeline() {
    auto vert_shader_code = SeUtil::readFile("Shader/Vert.spv");
    auto frag_shader_code = SeUtil::readFile("Shader/Frag.spv");
    m_vert_shader_module = createShaderModule(vert_shader_code);
    m_frag_shader_module = createShaderModule(frag_shader_code);
    qDebug() << "Shader modules created";
    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = m_vert_shader_module;
    vert_shader_stage_info.pName = "main";
    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = m_frag_shader_module;
    frag_shader_stage_info.pName = "main";
    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swap_chain_extent.width;
    viewport.height = (float)m_swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swap_chain_extent;

    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;            // Optional
    pipeline_layout_info.pSetLayouts = nullptr;         // Optional
    pipeline_layout_info.pushConstantRangeCount = 0;    // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

    VkResult result;
    result = vkCreatePipelineLayout(m_logical_device, &pipeline_layout_info, nullptr, &m_pipeline_layout);
    if (result == VK_SUCCESS) {
        qDebug() << "Pipeline layout created";
    } else {
        qDebug() << "Failed to create pipeline layout";
    }
    assert(result == VK_SUCCESS);

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = nullptr; // Optional
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = m_pipeline_layout;
    pipeline_info.renderPass = m_render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1;              // Optional

    result = vkCreateGraphicsPipelines(m_logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline);
    if (result == VK_SUCCESS) {
        qDebug() << "Pipeline created";
    } else {
        qDebug() << "Failed to create pipeline";
    }
    assert(result == VK_SUCCESS);
}

void SeVulkanWindow::destroyGraphicsPipeline() {
    if (m_graphics_pipeline) {
        vkDestroyPipeline(m_logical_device, m_graphics_pipeline, nullptr);
        m_graphics_pipeline = VK_NULL_HANDLE;
    }
    qDebug() << "Pipeline destroyed";
    if (m_pipeline_layout) {
        vkDestroyPipelineLayout(m_logical_device, m_pipeline_layout, nullptr);
        m_pipeline_layout = VK_NULL_HANDLE;
    }
    qDebug() << "Pipeline layout destroyed";
    if (m_frag_shader_module) {
        vkDestroyShaderModule(m_logical_device, m_frag_shader_module, nullptr);
        m_frag_shader_module = VK_NULL_HANDLE;
    }
    if (m_vert_shader_module) {
        vkDestroyShaderModule(m_logical_device, m_vert_shader_module, nullptr);
        m_vert_shader_module = VK_NULL_HANDLE;
    }
    qDebug() << "Shader modules destroyed";
}

VkShaderModule SeVulkanWindow::createShaderModule(std::vector<char> code) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkResult result;
    result = vkCreateShaderModule(m_logical_device, &create_info, nullptr, &shader_module);
    if (result == VK_SUCCESS) {
    } else {
        qDebug() << "Failed to create shader module!";
    }
    assert(result == VK_SUCCESS);
    return shader_module;
}

#pragma endregion Graphics pipeline