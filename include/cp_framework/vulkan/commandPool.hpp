#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <vector>
#include <span>

namespace cp::vulkan
{
    class CommandPool;
    struct CommandBuffer
    {
        friend class CommandPool;

    private:
        CommandBuffer(const uint32_t &index) : m_index(index) {}
        const uint32_t m_index;
    };

    class Device;
    class CommandPool
    {
    public:
        CommandPool(Device &device, const uint32_t &queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        ~CommandPool();

        CP_RULE_OF_FIVE_DELETE(CommandPool);
        CP_HANDLE_CONVERSION(VkCommandPool, m_commandPool);

        CommandBuffer CreateCommandBuffer(const VkCommandBufferLevel &level);
        const VkCommandBuffer &GetCommandBuffer(const CommandBuffer &commandBuffer);

    private:
        Device &m_device;
        VkCommandPool m_commandPool;

        std::vector<VkCommandBuffer> m_commandBuffers;
    };
} // namespace cp
