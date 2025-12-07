#include "cp_framework/vulkan/utils.hpp"
#include "cp_framework/debug/debug.hpp"
#include <set>

namespace cp::vulkan::utils
{
    VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT pfunc)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = pfunc;
        return createInfo;
    }

    std::vector<const char *> GetGLFWRequiredExtensions(bool validationLayersEnabled, std::span<const char *> aditionalRequiredExtensions)
    {
        uint32_t count = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

        if (!glfwExtensions)
            LOG_THROW("GLFW NOT INITIALIZED OR FAILED TO OBTAIN REQUIRED EXTENSIONS");

        std::vector<const char *> ext(glfwExtensions, glfwExtensions + count);

        if (validationLayersEnabled)
            ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        ext.insert(ext.end(), aditionalRequiredExtensions.begin(), aditionalRequiredExtensions.end());

        return ext;
    }

    bool checkValidationLayerSupport(std::span<const char *> validationLayers)
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    bool isDeviceSuitable(VkInstance instance, VkPhysicalDevice device, VkSurfaceKHR surface, std::span<const char *> deviceExtensions)
    {
        VkPhysicalDeviceProperties2 deviceProp{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(device, &deviceProp);

        VkPhysicalDeviceFeatures2 deviceFeat{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        vkGetPhysicalDeviceFeatures2(device, &deviceFeat);

        QueueFamilyIndices indices = findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(instance, device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        bool isDiscreteOrIntegrated =
            deviceProp.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
            deviceProp.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

        return isDiscreteOrIntegrated && indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices;

        // --- 1) Query queue family properties (use *2 to allow pNext extensions) ---
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount);
        for (auto &q : queueFamilies)
        {
            q.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
            q.pNext = nullptr;
        }
        vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, queueFamilies.data());

        // --- 2) Query device features for sparse binding and protected memory support ---
        VkPhysicalDeviceFeatures2 features2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = nullptr};
        VkPhysicalDeviceProtectedMemoryFeatures protectedFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES, .pNext = nullptr};
        features2.pNext = &protectedFeatures;
        vkGetPhysicalDeviceFeatures2(device, &features2);

        bool supportsSparseBinding = features2.features.sparseBinding == VK_TRUE;
        bool supportsProtectedMemory = protectedFeatures.protectedMemory == VK_TRUE;

        // --- 3) Iterate queue families and pick candidates ---
        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            const auto &props = queueFamilies[i].queueFamilyProperties;
            VkQueueFlags flags = props.queueFlags;

            // --- Present support (surface) ---
            VkBool32 presentSupport = VK_FALSE;
            if (surface != VK_NULL_HANDLE)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            }

            // NOTE: some drivers expose whether a queue supports protected submissions via a
            // per-family property in the pNext chain of VkQueueFamilyProperties2.
            // To be robust, you can attempt to read that structure here (example below).
            bool familySupportsProtected = false;
            if (supportsProtectedMemory)
            {
                // Example: many Vulkan SDKs provide VkQueueFamilyProtectedProperties / KHR equivalent.
                // Try to read it from the pNext chain of queueFamilies[i] if available.
                // We'll use a generic approach (safe-cast) — if your build has the struct, you can
                // query it explicitly; otherwise leave familySupportsProtected = false.
                VkQueueFamilyProperties2 qf2 = queueFamilies[i];
                // If your SDK has VkQueueFamilyProtectedProperties, you can use it like:
                // VkQueueFamilyProtectedProperties protectedProps{ .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROTECTED_PROPERTIES };
                // protectedProps.pNext = nullptr;
                // qf2.pNext = &protectedProps;
                // vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, queueFamilies.data());
                // familySupportsProtected = (protectedProps.protectedSubmit == VK_TRUE);
                // For portability here, we'll leave as false unless you enable the query above.
            }

            // --- Graphics + Present ---
            if (flags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (!indices.graphicsFamily.has_value())
                    indices.graphicsFamily = i;
            }
            if (presentSupport && !indices.presentFamily.has_value())
                indices.presentFamily = i;

            // --- Compute-only (prefer exclusive) ---
            if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT))
            {
                indices.computeFamily = i;
            }

            // --- Transfer-only (prefer exclusive) ---
            if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)))
            {
                indices.transferFamily = i;
            }
        }

        // --- 4) Fallback chain to ensure all fields are filled ---
        // If the system has only one family, these will all resolve to the same index.
        // --- Fallbacks ---
        if (!indices.computeFamily.has_value())
        {
            // fallback: use any compute queue
            for (uint32_t i = 0; i < queueFamilyCount; ++i)
            {
                if (queueFamilies[i].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    indices.computeFamily = i;
                    break;
                }
            }
        }

        if (!indices.transferFamily.has_value())
        {
            // fallback: use any transfer queue
            for (uint32_t i = 0; i < queueFamilyCount; ++i)
            {
                if (queueFamilies[i].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    indices.transferFamily = i;
                    break;
                }
            }
        }

        if (!indices.presentFamily.has_value())
            indices.presentFamily = indices.graphicsFamily;

        return indices;
    }

    SwapChainSupportDetails querySwapChainSupport(VkInstance instance, VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
            .pNext = nullptr,
            .surface = surface};

        SwapChainSupportDetails details{};
        details.capabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
        details.capabilities.pNext = nullptr;

        VkResult result = VK_SUCCESS;

        // --- Surface Capabilities ---
        if (auto func = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR")))
        {
            result = vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surfaceInfo, &details.capabilities);
        }
        else
        {
            result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities.surfaceCapabilities);
        }

        if (result != VK_SUCCESS)
        {
            LOG_THROW("SwapChain query failed with VkResult = {}", std::to_string(result));
        }

        // --- Surface Formats ---
        uint32_t formatCount = 0;
        if (auto func = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormats2KHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR")))
        {
            result = vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &formatCount, nullptr);
            if (formatCount != 0)
            {
                details.formats.resize(formatCount);
                for (auto &fmt : details.formats)
                {
                    fmt.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
                    fmt.pNext = nullptr;
                }
                result = vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &formatCount, details.formats.data());
            }
        }
        else
        {
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            if (formatCount != 0)
            {
                std::vector<VkSurfaceFormatKHR> formats(formatCount);
                result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
                details.formats.resize(formatCount);
                for (uint32_t i = 0; i < formatCount; ++i)
                {
                    details.formats[i].surfaceFormat = formats[i];
                    details.formats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
                    details.formats[i].pNext = nullptr;
                }
            }
        }

        if (result != VK_SUCCESS)
        {
            LOG_THROW("SwapChain query failed with VkResult = {}", std::to_string(result));
        }

        // --- Present Modes ---
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char *> deviceExtensions)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    void logSelectedGPU(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        std::string typeStr;
        switch (props.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            typeStr = "Discrete GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            typeStr = "Integrated GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            typeStr = "Virtual GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            typeStr = "CPU (Software Rasterizer)";
            break;
        default:
            typeStr = "Other/Unknown";
            break;
        }

        uint32_t apiVersion = props.apiVersion;
        uint32_t apiMajor = VK_VERSION_MAJOR(apiVersion);
        uint32_t apiMinor = VK_VERSION_MINOR(apiVersion);
        uint32_t apiPatch = VK_VERSION_PATCH(apiVersion);

        uint32_t driverVersion = props.driverVersion;

        LOG_INFO("============================================================");
        LOG_INFO("[ GPU SELECIONADA ]");
        LOG_INFO("  Nome:                 {}", props.deviceName);
        LOG_INFO("  Tipo:                 {}", typeStr);
        LOG_INFO("  Vulkan API Version:   {}.{}.{}", apiMajor, apiMinor, apiPatch);
        LOG_INFO("  Driver Version:       {}", driverVersion);
        LOG_INFO("  Vendor ID:            0x{:04X}", props.vendorID);
        LOG_INFO("  Device ID:            0x{:04X}", props.deviceID);
        LOG_INFO("============================================================");

        LOG_INFO("[ LIMITES DO DISPOSITIVO ]");
        LOG_INFO("  Max Image 2D:                {}", props.limits.maxImageDimension2D);
        LOG_INFO("  Max Bound Descriptor Sets:   {}", props.limits.maxBoundDescriptorSets);
        LOG_INFO("  Max Push Constants:          {} bytes", props.limits.maxPushConstantsSize);
        LOG_INFO("============================================================");

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);

        LOG_INFO("[ MEMORIA ]");
        LOG_INFO("  Heaps encontrados: {}", memProps.memoryHeapCount);
        for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i)
        {
            const auto &heap = memProps.memoryHeaps[i];
            float sizeGB = heap.size / (1024.0f * 1024.0f * 1024.0f);
            LOG_INFO("    Heap {:>2}: {:>6.2f} GB ({})",
                     i, sizeGB,
                     (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) ? "Device Local" : "Host Visible");
        }
        LOG_INFO("============================================================");
    }

    void logDeviceFeatures(
        const VkPhysicalDeviceFeatures2 &supported,
        const VkPhysicalDeviceVulkan11Features &supported11,
        const VkPhysicalDeviceVulkan12Features &supported12,
        const VkPhysicalDeviceVulkan13Features &supported13,
        const VkPhysicalDeviceFeatures2 &enabled,
        const VkPhysicalDeviceVulkan11Features &enabled11,
        const VkPhysicalDeviceVulkan12Features &enabled12,
        const VkPhysicalDeviceVulkan13Features &enabled13)
    {
        LOG_INFO("===== Vulkan Device Feature Report =====");

        auto logFeature = [](const char *name, VkBool32 supported, VkBool32 enabled)
        {
            std::string status = supported ? (enabled ? "ENABLED" : "AVAILABLE") : "UNSUPPORTED";
            LOG_INFO("  {:<35} {}", name, status);
        };

        LOG_INFO(">> Vulkan 1.0 Features");
        logFeature("samplerAnisotropy", supported.features.samplerAnisotropy, enabled.features.samplerAnisotropy);
        logFeature("sampleRateShading", supported.features.sampleRateShading, enabled.features.sampleRateShading);
        logFeature("fillModeNonSolid", supported.features.fillModeNonSolid, enabled.features.fillModeNonSolid);
        logFeature("wideLines", supported.features.wideLines, enabled.features.wideLines);
        logFeature("geometryShader", supported.features.geometryShader, enabled.features.geometryShader);
        logFeature("tessellationShader", supported.features.tessellationShader, enabled.features.tessellationShader);

        LOG_INFO(">> Vulkan 1.1 Features");
        logFeature("multiview", supported11.multiview, enabled11.multiview);
        logFeature("protectedMemory", supported11.protectedMemory, enabled11.protectedMemory);
        logFeature("samplerYcbcrConversion", supported11.samplerYcbcrConversion, enabled11.samplerYcbcrConversion);
        logFeature("shaderDrawParameters", supported11.shaderDrawParameters, enabled11.shaderDrawParameters);

        LOG_INFO(">> Vulkan 1.2 Features");
        logFeature("scalarBlockLayout", supported12.scalarBlockLayout, enabled12.scalarBlockLayout);
        logFeature("descriptorIndexing", supported12.descriptorIndexing, enabled12.descriptorIndexing);
        logFeature("imagelessFramebuffer", supported12.imagelessFramebuffer, enabled12.imagelessFramebuffer);
        logFeature("uniformBufferStandardLayout", supported12.uniformBufferStandardLayout, enabled12.uniformBufferStandardLayout);
        logFeature("separateDepthStencilLayouts", supported12.separateDepthStencilLayouts, enabled12.separateDepthStencilLayouts);
        logFeature("hostQueryReset", supported12.hostQueryReset, enabled12.hostQueryReset);
        logFeature("timeline semaphore", supported12.timelineSemaphore, enabled12.timelineSemaphore);

        LOG_INFO(">> Vulkan 1.3 Features");
        logFeature("dynamicRendering", supported13.dynamicRendering, enabled13.dynamicRendering);
        logFeature("synchronization2", supported13.synchronization2, enabled13.synchronization2);
        logFeature("maintenance4", supported13.maintenance4, enabled13.maintenance4);

        LOG_INFO("========================================");
    }

    VkSurfaceFormat2KHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {

            if (availableFormat.surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes, VkPresentModeKHR preferredMode)
    {
        if (std::find(availablePresentModes.begin(), availablePresentModes.end(), preferredMode) != availablePresentModes.end())
        {
            return preferredMode;
        }

        for (const auto &availablePresentMode : availablePresentModes)
        {

            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilities2KHR &capabilities)
    {
        if (capabilities.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.surfaceCapabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

            actualExtent.width = std::clamp(actualExtent.width, capabilities.surfaceCapabilities.minImageExtent.width, capabilities.surfaceCapabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.surfaceCapabilities.minImageExtent.height, capabilities.surfaceCapabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    VkFormat findSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        LOG_THROW("Failed to find suitable format!");
        return VK_FORMAT_UNDEFINED;
    }

    VkFormat findDepthFormat(VkPhysicalDevice physDevice)
    {
        return findSupportedFormat(physDevice,
                                   {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool hasStencilFormat(const VkFormat &format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

} // namespace cp::vulkan::utils
