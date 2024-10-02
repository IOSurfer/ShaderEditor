#ifndef SE_SWAP_CHAIN_SUPPORT_DETAILS_H
#define SE_SWAP_CHAIN_SUPPORT_DETAILS_H

#include <vector>
#include <vulkan/vulkan.h>

struct SeSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

    bool isSwapChainAdequate() {
        return !formats.empty() && !present_modes.empty();
    };
};

#endif