#pragma once

#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include "vkTypes.hpp"
#include <vector>
#include <span>

namespace cp::vulkan::utils
{
    VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT pfunc);
    std::vector<const char *> GetGLFWRequiredExtensions(bool validationLayersEnabled, std::span<const char *> aditionalRequiredExtensions);
    bool checkValidationLayerSupport(std::span<const char *> validationLayers);

    bool isDeviceSuitable(VkInstance instance, VkPhysicalDevice device, VkSurfaceKHR surface, std::span<const char *> deviceExtensions);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    SwapChainSupportDetails querySwapChainSupport(VkInstance instance, VkPhysicalDevice device, VkSurfaceKHR surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char *> deviceExtensions);

    VkSurfaceFormat2KHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes, VkPresentModeKHR preferredMode);
    VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilities2KHR &capabilities);
    VkFormat findSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat(VkPhysicalDevice physDevice);
    bool hasStencilFormat(const VkFormat &format);

    void logSelectedGPU(VkPhysicalDevice device);

    void logDeviceFeatures(
        const VkPhysicalDeviceFeatures2 &supported,
        const VkPhysicalDeviceVulkan11Features &supported11,
        const VkPhysicalDeviceVulkan12Features &supported12,
        const VkPhysicalDeviceVulkan13Features &supported13,
        const VkPhysicalDeviceFeatures2 &enabled,
        const VkPhysicalDeviceVulkan11Features &enabled11,
        const VkPhysicalDeviceVulkan12Features &enabled12,
        const VkPhysicalDeviceVulkan13Features &enabled13);

    void SignalTimelineSemaphore(VkDevice device, VkSemaphore semaphore, const uint64_t &value);
    void WaitTimelineSemaphores(VkDevice device, std::span<const VkSemaphore> semaphores, std::span<uint64_t> &values, const uint64_t &timeout = UINT64_MAX);

    VkResult BeginCommandBuffer(VkCommandBuffer cmdBuffer,
                                const std::vector<VkFormat> &colorAttachments,
                                const VkFormat &depthFormat,
                                const VkFormat &stencilFormat,
                                const VkSampleCountFlagBits &rasterizationSamples);
}