#include "cp_framework/vulkan/physicalDevice.hpp"
#include "cp_framework/vulkan/instance.hpp"
#include "cp_framework/vulkan/utils.hpp"
#include "cp_framework/vulkan/surface.hpp"
#include "cp_framework/debug/debug.hpp"
namespace cp::vulkan
{
    PhysicalDevice::PhysicalDevice(Instance &instance, Surface &surface, std::span<const char *> deviceExtensions)
        : m_instance(instance)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance.get(), &deviceCount, nullptr);

        if (deviceCount == 0)
            LOG_THROW("Failed to enumerate physical devices!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance.get(), &deviceCount, devices.data());

        for (const auto &device : devices)
        {
            if (utils::isDeviceSuitable(m_instance.get(), device, surface.get(), deviceExtensions))
            {
                m_physDevice = device;
                break;
            }
        }

        if (m_physDevice == VK_NULL_HANDLE)
        {
            LOG_THROW("Failed to find a suitable GPU!");
        }

        utils::logSelectedGPU(m_physDevice);
    }
}