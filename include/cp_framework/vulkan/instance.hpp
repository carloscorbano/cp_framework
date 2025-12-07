#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <vector>
#include <span>

namespace cp::vulkan
{
    class Instance
    {
    public:
        Instance(bool enabledValidationLayers, std::span<const char *> aditionalRequiredExtensions, std::span<const char *> validationLayers, PFN_vkDebugUtilsMessengerCallbackEXT pfunc);
        ~Instance();

        CP_RULE_OF_FIVE_DELETE(Instance);
        CP_HANDLE_CONVERSION(VkInstance, m_instance);

    private:
        VkInstance m_instance = VK_NULL_HANDLE;
    };
}