#include "cp_framework/vulkan/commandPool.hpp"
#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    CommandBuffer::CommandBuffer(Device &device, CommandPool &pool, VkCommandBufferLevel &level)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pool.get();
        allocInfo.level = level;
        allocInfo.pNext = nullptr;

        if (vkAllocateCommandBuffers(device.get(), &allocInfo, &m_cmdBuffer) != VK_SUCCESS)
        {
            LOG_THROW("Failed to allocate command buffer!");
        }
    }

    CommandPool::CommandPool(Device &device, const uint32_t &queueFamilyIndex, VkCommandPoolCreateFlags flags)
        : m_device(device)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.pNext = nullptr;
        poolInfo.flags = flags;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        if (vkCreateCommandPool(device.get(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        {
            LOG_THROW("Failed to create command pool!");
        }
    }

    CommandPool::~CommandPool()
    {
        CP_VK_DESTROY(m_device.get(), m_commandPool, vkDestroyCommandPool);
    }
}