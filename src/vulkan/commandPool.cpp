#include "cp_framework/vulkan/commandPool.hpp"
#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
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
        CP_VK_DELETE_HANDLE(m_commandPool, vkDestroyCommandPool(m_device.get(), m_commandPool, nullptr));
    }

    CommandBuffer CommandPool::CreateCommandBuffer(const VkCommandBufferLevel &level)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = level;
        allocInfo.pNext = nullptr;

        VkCommandBuffer cmdb;
        if (vkAllocateCommandBuffers(m_device.get(), &allocInfo, &cmdb) != VK_SUCCESS)
        {
            LOG_THROW("Failed to allocate command buffer!");
        }

        uint32_t index = static_cast<uint32_t>(m_commandBuffers.size());
        m_commandBuffers.push_back(std::move(cmdb));

        return {index};
    }

    const VkCommandBuffer &CommandPool::GetCommandBuffer(const CommandBuffer &commandBuffer)
    {
        return m_commandBuffers.at(commandBuffer.m_index);
    }
}