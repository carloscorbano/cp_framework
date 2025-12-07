#pragma once

#include <optional>
#include <cstdint>
#include <vector>
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"

#define VK_API_VERSION VK_API_VERSION_1_3

namespace cp::vulkan
{
    enum class QueueType
    {
        GRAPHICS,
        PRESENT,
        COMPUTE,
        TRANSFER
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> transferFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value(); }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilities2KHR capabilities;
        std::vector<VkSurfaceFormat2KHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct DeviceQueues
    {
        VkQueue graphics = VK_NULL_HANDLE;
        VkQueue present = VK_NULL_HANDLE;
        VkQueue compute = VK_NULL_HANDLE;
        VkQueue transfer = VK_NULL_HANDLE;
    };
} // namespace cp::vulkan
