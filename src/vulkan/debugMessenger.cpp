#include "cp_framework/vulkan/debugMessenger.hpp"
#include "cp_framework/vulkan/utils.hpp"
#include "cp_framework/debug/debug.hpp"
#include "cp_framework/vulkan/instance.hpp"

namespace cp::vulkan
{
    DebugMessenger::DebugMessenger(Instance &instance, PFN_vkDebugUtilsMessengerCallbackEXT pfunc)
        : m_instance(instance)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = utils::DebugMessengerCreateInfo(pfunc);
        auto createFunc = [](VkInstance instance,
                             const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkDebugUtilsMessengerEXT *pDebugMessenger) -> VkResult
        {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            }
            else
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        };

        if (createFunc(instance.get(), &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            LOG_THROW("[VULKAN] Failed to create debug messenger!");
        }
    }

    DebugMessenger::~DebugMessenger()
    {
        if (m_debugMessenger == VK_NULL_HANDLE)
            return;

        auto destroyMessenger = [](VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) -> void
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                func(instance, debugMessenger, pAllocator);
            }
        };

        destroyMessenger(m_instance.get(), m_debugMessenger, nullptr);
        m_debugMessenger = VK_NULL_HANDLE;
    }
}