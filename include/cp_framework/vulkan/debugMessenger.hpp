#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"

namespace cp::vulkan
{
    class Instance;
    class DebugMessenger
    {
    public:
        DebugMessenger(Instance &instance, PFN_vkDebugUtilsMessengerCallbackEXT pfunc);
        ~DebugMessenger();

        CP_NO_COPY_CLASS(DebugMessenger);

        CP_HANDLE_CONVERSION(VkDebugUtilsMessengerEXT, m_debugMessenger);

    private:
        Instance &m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
    };
} // namespace cp::vulkan
