#include "cp_framework/vulkan/instance.hpp"
#include "cp_framework/vulkan/utils.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    Instance::Instance(bool enabledValidationLayers, std::span<const char *> aditionalRequiredExtensions, std::span<const char *> validationLayers, PFN_vkDebugUtilsMessengerCallbackEXT pfunc)
    {
        if (enabledValidationLayers && !utils::checkValidationLayerSupport(validationLayers))
        {
            LOG_THROW("[VULKAN] Validation layers required but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "cpgame";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "cpframework";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION;

        VkInstanceCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pNext = nullptr};
        createInfo.pApplicationInfo = &appInfo;

        auto reqExtensions = utils::GetGLFWRequiredExtensions(enabledValidationLayers, aditionalRequiredExtensions);

        createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size());
        createInfo.ppEnabledExtensionNames = reqExtensions.data();
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        for (auto ext : reqExtensions)
            LOG_INFO("Instance extension enabled: {}", ext);

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = utils::DebugMessengerCreateInfo(pfunc);

        if (enabledValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        {
            LOG_THROW("[VULKAN] Failed to create vulkan instance!");
        }
    }

    Instance::~Instance()
    {
        CP_VK_DELETE_HANDLE(m_instance, vkDestroyInstance(m_instance, nullptr));
    }
}