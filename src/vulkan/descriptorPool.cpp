#include "cp_framework/vulkan/descriptorPool.hpp"
#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    DescriptorPool::DescriptorPool(Device &device, uint32_t maxSets, std::span<const VkDescriptorPoolSize> sizes, VkDescriptorPoolCreateFlags flags)
        : m_device(device)
    {
        VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        info.poolSizeCount = (uint32_t)sizes.size();
        info.pPoolSizes = sizes.data();
        info.maxSets = maxSets;
        info.flags = flags;

        if (vkCreateDescriptorPool(device.get(), &info, nullptr, &m_descriptorPool) != VK_SUCCESS)
            LOG_THROW("Failed to create descriptor pool!");
    }

    DescriptorPool::~DescriptorPool()
    {
        destroy();
    }

    void DescriptorPool::destroy()
    {
        CP_VK_DESTROY(m_device.get(), m_descriptorPool, vkDestroyDescriptorPool);
    }
}