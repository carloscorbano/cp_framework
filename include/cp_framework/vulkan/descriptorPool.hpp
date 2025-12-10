#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <span>

namespace cp::vulkan
{
    class Device;
    class DescriptorPool
    {
    public:
        DescriptorPool(Device &device, uint32_t maxSets, std::span<const VkDescriptorPoolSize> sizes, VkDescriptorPoolCreateFlags flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        ~DescriptorPool();

        CP_RULE_OF_FIVE_DELETE(DescriptorPool);
        CP_HANDLE_CONVERSION(VkDescriptorPool, m_descriptorPool);

    private:
        void destroy();

    private:
        Device &m_device;
        VkDescriptorPool m_descriptorPool;
    };
} // namespace cp::vulkan
