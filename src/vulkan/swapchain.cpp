#include "cp_framework/vulkan/swapchain.hpp"
#include "cp_framework/vulkan/instance.hpp"
#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/vulkan/physicalDevice.hpp"
#include "cp_framework/vulkan/surface.hpp"
#include "cp_framework/vulkan/utils.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    Swapchain::Swapchain(GLFWwindow *window, Instance &instance, Device &device, PhysicalDevice &physDevice, Surface &surface, VkPresentModeKHR preferredMode, Swapchain *oldSwapchain)
        : m_device(device)
    {
        SwapChainSupportDetails swapChainSupport = utils::querySwapChainSupport(instance.get(), physDevice.get(), surface.get());

        VkSurfaceFormat2KHR format = utils::chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = utils::chooseSwapPresentMode(swapChainSupport.presentModes, preferredMode);
        VkExtent2D extent = utils::chooseSwapExtent(window, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.surfaceCapabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.surfaceCapabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.surfaceCapabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.surfaceCapabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface.get();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format.surfaceFormat.format;
        createInfo.imageColorSpace = format.surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        auto familyIndices = device.GetIndices();

        uint32_t queueFamilyIndices[] = {familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value()};

        if (familyIndices.graphicsFamily != familyIndices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;     // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = oldSwapchain ? oldSwapchain->get() : VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device.get(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        {
            LOG_THROW("Failed to create swap chain!");
        }

        // Helper functions to convert Vulkan enums to strings
        auto vkFormatToString = [](VkFormat format) -> std::string
        {
            switch (format)
            {
            case VK_FORMAT_B8G8R8A8_SRGB:
                return "VK_FORMAT_B8G8R8A8_SRGB";
            case VK_FORMAT_B8G8R8A8_UNORM:
                return "VK_FORMAT_B8G8R8A8_UNORM";
            case VK_FORMAT_R8G8B8A8_SRGB:
                return "VK_FORMAT_R8G8B8A8_SRGB";
            case VK_FORMAT_R8G8B8A8_UNORM:
                return "VK_FORMAT_R8G8B8A8_UNORM";
            default:
                return "UNKNOWN_FORMAT";
            }
        };

        auto vkColorSpaceToString = [](VkColorSpaceKHR colorSpace) -> std::string
        {
            switch (colorSpace)
            {
            case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
                return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
            case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
                return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT";
            case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
                return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
            default:
                return "UNKNOWN_COLOR_SPACE";
            }
        };

        auto vkPresentModeToString = [](VkPresentModeKHR presentMode) -> std::string
        {
            switch (presentMode)
            {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                return "VK_PRESENT_MODE_IMMEDIATE_KHR";
            case VK_PRESENT_MODE_MAILBOX_KHR:
                return "VK_PRESENT_MODE_MAILBOX_KHR";
            case VK_PRESENT_MODE_FIFO_KHR:
                return "VK_PRESENT_MODE_FIFO_KHR";
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
            default:
                return "UNKNOWN_PRESENT_MODE";
            }
        };

        // Log swap chain details
        LOG_INFO("============================================================");
        LOG_INFO("[ SWAPCHAIN CONFIG ]");
        LOG_INFO("  Format:           {}", vkFormatToString(format.surfaceFormat.format));
        LOG_INFO("  Color Space:      {}", vkColorSpaceToString(format.surfaceFormat.colorSpace));
        LOG_INFO("  Present Mode:     {}", vkPresentModeToString(presentMode));
        LOG_INFO("  Image Count:      {}", imageCount);
        LOG_INFO("  Extent:           {}x{}", extent.width, extent.height);
        LOG_INFO("============================================================");

        vkGetSwapchainImagesKHR(device.get(), m_swapchain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(device.get(), m_swapchain, &imageCount, m_images.data());

        m_colorFormat = format.surfaceFormat.format;
        m_depthFormat = utils::findDepthFormat(physDevice.get());
        m_stencilFormat = utils::hasStencilFormat(m_depthFormat) ? m_depthFormat : VK_FORMAT_UNDEFINED;
        m_extent = extent;

        m_views.resize(m_images.size());
        for (size_t i = 0; i < m_images.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_colorFormat;
            viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.get(), &viewInfo, nullptr, &m_views[i]) != VK_SUCCESS)
            {
                LOG_THROW("Failed to create image views!");
            }
        }
    }

    Swapchain::~Swapchain()
    {
        auto device = m_device.get();
        CP_VK_DELETE_HANDLE(m_swapchain, vkDestroySwapchainKHR(device, m_swapchain, nullptr));

        for (auto &view : m_views)
        {
            CP_VK_DELETE_HANDLE(view, vkDestroyImageView(device, view, nullptr));
        }
    }
} // namespace cp::vulkan
