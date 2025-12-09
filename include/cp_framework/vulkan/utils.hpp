#pragma once

#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include "vkTypes.hpp"
#include <vector>
#include <span>

namespace cp::vulkan::utils
{
    /**
     * @brief Creates a debug messenger configuration using a custom callback function.
     *
     * @param pfunc Pointer to the debug callback function.
     * @return VkDebugUtilsMessengerCreateInfoEXT Configured debug messenger info struct.
     */
    VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT pfunc);

    /**
     * @brief Retrieves required Vulkan extensions based on GLFW and optional extra extensions.
     *
     * @param validationLayersEnabled Whether validation layers will be used (adds debug extension).
     * @param aditionalRequiredExtensions Additional extensions requested by the application.
     * @return std::vector<const char*> List of required Vulkan extensions.
     */
    std::vector<const char *> GetGLFWRequiredExtensions(bool validationLayersEnabled,
                                                        std::span<const char *> aditionalRequiredExtensions);

    /**
     * @brief Checks if all requested validation layers are supported by the Vulkan implementation.
     *
     * @param validationLayers Validation layers to check.
     * @return true if all validation layers are supported.
     * @return false otherwise.
     */
    bool checkValidationLayerSupport(std::span<const char *> validationLayers);

    /**
     * @brief Determines whether a GPU meets all required capabilities and extensions.
     *
     * @param instance Vulkan instance.
     * @param device Physical device to evaluate.
     * @param surface Rendering surface for swapchain support checks.
     * @param deviceExtensions List of required device extensions.
     * @return true if the GPU is suitable for rendering.
     * @return false otherwise.
     */
    bool isDeviceSuitable(VkInstance instance,
                          VkPhysicalDevice device,
                          VkSurfaceKHR surface,
                          std::span<const char *> deviceExtensions);

    /**
     * @brief Finds queue families required for graphics, compute, transfer, and presentation.
     *
     * @param device Physical device to scan.
     * @param surface Presentation surface.
     * @return QueueFamilyIndices Struct containing located queue family indices.
     */
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    /**
     * @brief Queries swapchain surface capabilities, formats, and present modes.
     *
     * @param instance Vulkan instance.
     * @param device Physical device.
     * @param surface Rendering surface.
     * @return SwapChainSupportDetails Detailed information about swapchain support.
     */
    SwapChainSupportDetails querySwapChainSupport(VkInstance instance,
                                                  VkPhysicalDevice device,
                                                  VkSurfaceKHR surface);

    /**
     * @brief Checks whether the GPU supports all required device extensions.
     *
     * @param device Physical device being evaluated.
     * @param deviceExtensions Required extension names.
     * @return true if the device supports all extensions.
     * @return false otherwise.
     */
    bool checkDeviceExtensionSupport(VkPhysicalDevice device,
                                     std::span<const char *> deviceExtensions);

    /**
     * @brief Selects the most appropriate surface format from available options.
     *
     * @param availableFormats Formats supported by the surface.
     * @return VkSurfaceFormat2KHR Selected surface format.
     */
    VkSurfaceFormat2KHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR> &availableFormats);

    /**
     * @brief Selects the preferred present mode, falling back if unavailable.
     *
     * @param availablePresentModes List of present modes supported by the surface.
     * @param preferredMode Requested present mode.
     * @return VkPresentModeKHR Chosen present mode.
     */
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes,
                                           VkPresentModeKHR preferredMode);

    /**
     * @brief Chooses the swapchain extent (resolution) based on window size and surface limits.
     *
     * @param window GLFW window used to query framebuffer size.
     * @param capabilities Surface capabilities for constraints.
     * @return VkExtent2D Selected extent in pixels.
     */
    VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilities2KHR &capabilities);

    /**
     * @brief Finds the first supported image format from a list of candidates.
     *
     * @param physDevice Physical device.
     * @param candidates Formats to evaluate.
     * @param tiling Required tiling mode.
     * @param features Required format features.
     * @return VkFormat First valid supported format.
     */
    VkFormat findSupportedFormat(VkPhysicalDevice physDevice,
                                 const std::vector<VkFormat> &candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features);

    /**
     * @brief Returns the first supported depth format from common candidates.
     *
     * @param physDevice Physical device.
     * @return VkFormat Depth format.
     */
    VkFormat findDepthFormat(VkPhysicalDevice physDevice);

    /**
     * @brief Checks whether a format includes stencil component data.
     *
     * @param format Format to evaluate.
     * @return true if the format supports stencil.
     * @return false otherwise.
     */
    bool hasStencilFormat(const VkFormat &format);

    /**
     * @brief Logs basic information about the selected physical device.
     *
     * @param device The chosen GPU.
     */
    void logSelectedGPU(VkPhysicalDevice device);

    /**
     * @brief Logs which GPU features are supported and which were enabled.
     *
     * Useful for debugging feature chains across Vulkan 1.0–1.3.
     */
    void logDeviceFeatures(
        const VkPhysicalDeviceFeatures2 &supported,
        const VkPhysicalDeviceVulkan11Features &supported11,
        const VkPhysicalDeviceVulkan12Features &supported12,
        const VkPhysicalDeviceVulkan13Features &supported13,
        const VkPhysicalDeviceFeatures2 &enabled,
        const VkPhysicalDeviceVulkan11Features &enabled11,
        const VkPhysicalDeviceVulkan12Features &enabled12,
        const VkPhysicalDeviceVulkan13Features &enabled13);

    /**
     * @brief Signals a timeline semaphore to a specified value.
     *
     * @param device Logical device.
     * @param semaphore Timeline semaphore.
     * @param value Value to signal.
     */
    void SignalTimelineSemaphore(VkDevice device, VkSemaphore semaphore, const uint64_t &value);

    /**
     * @brief Waits for multiple timeline semaphores to reach given values.
     *
     * @param device Logical device.
     * @param semaphores Semaphores to wait on.
     * @param values Expected values to wait for.
     * @param timeout Timeout in nanoseconds (default UINT64_MAX).
     */
    void WaitTimelineSemaphores(VkDevice device,
                                std::span<const VkSemaphore> semaphores,
                                std::span<uint64_t> &values,
                                const uint64_t &timeout = UINT64_MAX);

    /**
     * @brief Begins recording a command buffer and sets up initial rendering attachments.
     *
     * @param cmdBuffer Command buffer to begin.
     * @param colorAttachments List of color attachment formats.
     * @param depthFormat Depth buffer format.
     * @param stencilFormat Stencil buffer format.
     * @param rasterizationSamples MSAA sample count.
     * @return VkResult Vulkan status code.
     */
    VkResult BeginCommandBuffer(VkCommandBuffer cmdBuffer,
                                const std::vector<VkFormat> &colorAttachments,
                                const VkFormat &depthFormat,
                                const VkFormat &stencilFormat,
                                const VkSampleCountFlagBits &rasterizationSamples);

    /**
     * @brief Ends recording of a command buffer.
     *
     * @param cmdBuffer Command buffer to finalize.
     * @return VkResult Vulkan status code.
     */
    VkResult EndCommandBuffer(VkCommandBuffer cmdBuffer);

    /**
     * @brief Inserts a barrier that transitions an image from one layout to another.
     *
     * @param cmdBuffer Command buffer to record into.
     * @param image Image to transition.
     * @param format Pixel format of the image.
     * @param oldLayout Current layout.
     * @param newLayout Desired layout.
     */
    void TransitionImageLayout(VkCommandBuffer cmdBuffer,
                               VkImage image,
                               VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    /**
     * @brief Copies the contents of one image to another.
     *
     * @param commandBuffer Command buffer.
     * @param srcImage Source image.
     * @param dstImage Destination image.
     * @param width Copy width.
     * @param height Copy height.
     * @param mipLevel Mipmap level to copy.
     * @param layerCount Number of layers to copy.
     */
    void CopyImage(VkCommandBuffer commandBuffer,
                   VkImage srcImage,
                   VkImage dstImage,
                   uint32_t width,
                   uint32_t height,
                   uint32_t mipLevel,
                   uint32_t layerCount);
}
