#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include "vkTypes.hpp"
#include <span>

namespace cp::vulkan
{
    class PhysicalDevice;
    class Surface;
    class Device
    {
    public:
        Device(PhysicalDevice &physDevice, Surface &surface, bool validationLayersEnabled, std::span<const char *> validationLayers, std::span<const char *> deviceExtensions);
        ~Device();

        CP_RULE_OF_FIVE_DELETE(Device);
        CP_HANDLE_CONVERSION(VkDevice, m_device);

        const QueueFamilyIndices &GetIndices() noexcept { return m_familyIndices; }
        const DeviceQueues &GetQueues() noexcept { return m_familyQueues; }

    private:
        VkDevice m_device = VK_NULL_HANDLE;
        QueueFamilyIndices m_familyIndices;
        DeviceQueues m_familyQueues;
    };
} // namespace cp
