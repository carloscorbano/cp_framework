#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include "cp_framework/thirdparty/vma/vma.inc.hpp"

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>

namespace cp::vulkan
{
    /**
     * @brief Represents an asynchronous upload operation.
     *
     * Stores all Vulkan objects required to track a GPU-side transfer:
     * - Fence used to wait for completion.
     * - Command buffer that performed the transfer.
     * - Device and command pool used for submission.
     */
    struct UploadHandle
    {
        VkFence fence = VK_NULL_HANDLE;       ///< Completion fence.
        VkCommandBuffer cmd = VK_NULL_HANDLE; ///< Transfer command buffer.
        VkDevice device = VK_NULL_HANDLE;     ///< Vulkan device.
        VkCommandPool pool = VK_NULL_HANDLE;  ///< Pool from which the command buffer was allocated.

        /**
         * @brief Returns whether this handle represents a valid upload.
         */
        bool valid() const noexcept { return fence != VK_NULL_HANDLE || cmd != VK_NULL_HANDLE; }
    };

    /**
     * @brief Ring-buffer-based staging system for high-throughput Vulkan uploads.
     *
     * The StagingRing manages a persistently mapped GPU-visible buffer used as
     * a streaming upload area. Allocations advance a "head" pointer, while completed
     * uploads allow the "tail" pointer to catch up. A background thread ("janitor")
     * monitors pending submissions and frees ring space once transfers finish.
     *
     * Features:
     * - Zero-copy writes via persistent mapping.
     * - Lock-protected ring pointer management.
     * - Automatic tracking of upload completion.
     * - Threaded cleanup and tail advancement.
     */
    class StagingRing
    {
        /**
         * @brief Represents an allocated region inside the staging ring.
         */
        struct Reservation
        {
            VkDeviceSize offset; ///< Offset inside the ring buffer.
            void *ptr;           ///< CPU-visible pointer for writing.
        };

        /**
         * @brief Tracks a pending GPU transfer associated with a region of the ring.
         *
         * Once the transfer completes, the region can be safely overwritten.
         */
        struct Pending
        {
            UploadHandle handle;    ///< The upload operation to track.
            VkDeviceSize offsetEnd; ///< End offset of the reserved region.
        };

    public:
        /**
         * @brief Default constructor. Creates an empty/uninitialized staging ring.
         */
        StagingRing() = default;

        /**
         * @brief Creates a staging ring with the given total size and buffer usage flags.
         *
         * @param allocator VMA allocator used to create the staging buffer.
         * @param totalSize Size of the underlying ring buffer.
         * @param usage Vulkan buffer usage flags (default is transfer source).
         */
        StagingRing(VmaAllocator allocator, VkDeviceSize totalSize,
                    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        /**
         * @brief Destructor. Stops cleanup thread and frees all resources.
         */
        ~StagingRing();

        CP_NO_COPY_CLASS(StagingRing);

        /**
         * @brief Reserves a region of the ring buffer for writing.
         *
         * This method updates the head pointer and returns a writable region.
         * If the ring is full, it may block until space becomes available.
         *
         * @param size Number of bytes to reserve.
         * @param align Required alignment of the region.
         * @return Reservation Structure containing offset and mapped pointer.
         */
        Reservation Reserve(VkDeviceSize size, VkDeviceSize align = 16);

        /**
         * @brief Advances the tail pointer after completed uploads.
         *
         * The janitor thread calls this automatically, but it may also be called manually.
         *
         * @param newTail New tail position.
         */
        void AdvanceTailTo(VkDeviceSize newTail);

        /**
         * @brief Returns the Vulkan buffer used as the staging ring.
         */
        VkBuffer GetBuffer() const noexcept { return buffer; }

        /**
         * @brief Returns the total size of the staging ring (in bytes).
         */
        VkDeviceSize GetTotalSize() const noexcept { return totalSize; }

        /**
         * @brief Returns a persistently mapped CPU pointer to the ring buffer.
         */
        void *GetMappedPtr() const noexcept { return mapped; }

        /**
         * @brief Aligns a value upward to a required alignment.
         *
         * @param v Value to align.
         * @param align Alignment boundary.
         * @return VkDeviceSize Aligned value.
         */
        static VkDeviceSize Align(VkDeviceSize v, VkDeviceSize align);

        /**
         * @brief Returns the VMA allocation for the underlying buffer.
         */
        VmaAllocation getAllocation() const noexcept { return allocation; }

        /**
         * @brief Returns the VMA allocator used to create the staging buffer.
         */
        VmaAllocator getAllocator() const noexcept { return allocator; }

        /**
         * @brief Tracks an upload operation associated with a region of the ring.
         *
         * The janitor thread will monitor @p handle, and once the GPU finishes,
         * the corresponding region is released back to the ring.
         *
         * @param handle The asynchronous upload to track.
         * @param offset Start offset of the uploaded region.
         * @param size Size of the uploaded region.
         */
        void SubmitAndTrack(UploadHandle handle, VkDeviceSize offset, VkDeviceSize size);

    private:
        /**
         * @brief Creates the Vulkan buffer and persistent mapping.
         */
        void create();

        /**
         * @brief Destroys the Vulkan buffer, allocator resources, and stops cleanup threads.
         */
        void destroy();

        /**
         * @brief Starts the janitor thread responsible for monitoring upload completion.
         */
        void startJanitor();

        /**
         * @brief Signals the janitor thread to stop and waits for it to exit.
         */
        void stopJanitor();

        /**
         * @brief Thread loop that waits for uploads to complete and frees ring space.
         */
        void janitorLoop();

    private:
        VmaAllocator allocator = VK_NULL_HANDLE;   ///< VMA allocator.
        VkBuffer buffer = VK_NULL_HANDLE;          ///< Staging buffer.
        VmaAllocation allocation = VK_NULL_HANDLE; ///< VMA allocation for the buffer.
        VmaAllocationInfo allocationInfo{};        ///< Metadata describing the allocation.
        void *mapped = nullptr;                    ///< Persistently mapped pointer.
        VkDeviceSize totalSize = 0;                ///< Total size in bytes.
        VkBufferUsageFlags usage = 0;              ///< Vulkan usage flags.

        VkDeviceSize head = 0; ///< Next free position in the ring.
        VkDeviceSize tail = 0; ///< Oldest region not yet reclaimed.

        std::mutex commitMutex; ///< Protects head and tail pointers.

        std::mutex queueMutex;            ///< Protects the pending upload queue.
        std::condition_variable cv;       ///< Notifies the janitor thread.
        std::queue<Pending> pending;      ///< Pending upload operations.
        std::thread janitor;              ///< Background cleanup thread.
        std::atomic<bool> running{false}; ///< Indicates whether the janitor loop is active.
    };
} // namespace cp::vulkan
