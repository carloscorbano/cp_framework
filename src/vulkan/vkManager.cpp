#include "cp_framework/vulkan/vkManager.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp
{
    VkManager::VkManager(GLFWwindow *window)
        : m_instance(validationLayersEnabled, requiredExtensions, validationLayers, debugCallback),
          m_surface(window, m_instance),
          m_debugMessenger(m_instance, debugCallback),
          m_physDevice(m_instance, m_surface, deviceExtensions),
          m_device(m_physDevice, m_surface, validationLayersEnabled, validationLayers, deviceExtensions),
          m_vma(m_instance, m_device, m_physDevice),
          m_swapchain(window, m_instance, m_device, m_physDevice, m_surface, VK_PRESENT_MODE_FIFO_KHR)
    {
        ScopedLog("VULKAN MANAGER", "Starting to create vulkan manager class.", "Successfully created vulkan manager class.");
    }

    VkManager::~VkManager()
    {
        ScopedLog("VULKAN MANAGER", "Starting to destroy vulkan manager", "Successfully destroyed vulkan manager.");
    }
} // namespace cp

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    if (pCallbackData && pCallbackData->pMessage)
    {
        const char *severityStr = "UNKNOWN";
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            severityStr = "ERROR";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            severityStr = "WARNING";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            severityStr = "INFO";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            severityStr = "VERBOSE";
        }

        std::string typeStr;
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        {
            if (!typeStr.empty())
                typeStr += "/";
            typeStr += "GENERAL";
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            if (!typeStr.empty())
                typeStr += "/";
            typeStr += "VALIDATION";
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        {
            if (!typeStr.empty())
                typeStr += "/";
            typeStr += "PERFORMANCE";
        }
        if (typeStr.empty())
            typeStr = "UNKNOWN";

        std::string formatted = std::string("[VULKAN][") + severityStr + "][" + typeStr + "] " + pCallbackData->pMessage;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("{}", formatted);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARN("{}", formatted);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            LOG_INFO("{}", formatted);
        }
        else
        {
            LOG_DEBUG("{}", formatted);
        }
    }
    return VK_FALSE;
}