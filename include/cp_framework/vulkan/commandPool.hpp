#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <vector>
#include <span>

namespace cp::vulkan
{
    class Device;
    class CommandPool;

    /**
     * @brief Lightweight wrapper for a Vulkan command buffer.
     *
     * This struct stores a VkCommandBuffer handle and associates it
     * with its originating Device and CommandPool through construction.
     * It does not take ownership of the command buffer; destruction is
     * handled by the CommandPool.
     */
    struct CommandBuffer
    {
        /**
         * @brief Creates a command buffer wrapper.
         *
         * The actual allocation of the VkCommandBuffer is assumed to be
         * done externally (typically by CommandPool).
         *
         * @param device Reference to the Vulkan device.
         * @param pool Reference to the command pool from which it was allocated.
         * @param level The command buffer level (primary or secondary).
         */
        CommandBuffer(Device &device, CommandPool &pool, VkCommandBufferLevel &level);

        CP_HANDLE_CONVERSION(VkCommandBuffer, m_cmdBuffer);

    private:
        VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE; ///< Vulkan command buffer handle.
    };

    /**
     * @brief Wrapper for a Vulkan command pool.
     *
     * This class manages the creation and destruction of a VkCommandPool.
     * Command buffers allocated from this pool must be freed before the pool
     * is destroyed. The class does not automatically track individual command
     * buffers—users are responsible for managing allocations.
     */
    class CommandPool
    {
    public:
        /**
         * @brief Constructs a Vulkan command pool.
         *
         * @param device Reference to the Vulkan device.
         * @param queueFamilyIndex Queue family index this pool will allocate command buffers for.
         * @param flags Creation flags (e.g., VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT).
         */
        CommandPool(Device &device, const uint32_t &queueFamilyIndex,
                    VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        /**
         * @brief Destructor that destroys the Vulkan command pool and all its resources.
         */
        ~CommandPool();

        CP_RULE_OF_FIVE_DELETE(CommandPool);
        CP_HANDLE_CONVERSION(VkCommandPool, m_commandPool);

    private:
        Device &m_device;                             ///< Reference to the Vulkan device used to create the pool.
        VkCommandPool m_commandPool = VK_NULL_HANDLE; ///< Vulkan command pool handle.
    };
} // namespace cp::vulkan
