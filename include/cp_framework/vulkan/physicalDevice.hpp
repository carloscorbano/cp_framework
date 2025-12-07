#pragma once

#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include "cp_framework/core/types.hpp"
#include <span>

namespace cp::vulkan
{
    class Instance;
    class Surface;
    class PhysicalDevice
    {
    public:
        PhysicalDevice(Instance &instance, Surface &surface, std::span<const char *> deviceExtensions);
        ~PhysicalDevice() = default;

        CP_RULE_OF_FIVE_DELETE(PhysicalDevice);
        CP_HANDLE_CONVERSION(VkPhysicalDevice, m_physDevice);

    private:
        Instance &m_instance;
        VkPhysicalDevice m_physDevice;
    };
} // namespace cp::vulkan
