#pragma once

#include "stagingRing.hpp"

namespace cp::vulkan
{
    /**
     * @brief Wrapper class for a Vulkan buffer with VMA allocation and upload utilities.
     *
     * This class manages creation, allocation, mapping, data upload, and resizing
     * of Vulkan buffers. It optionally supports persistent mapping and integrates
     * with cp::vulkan::StagingRing for optimized streaming uploads.
     */
    class Buffer
    {
    public:
        /**
         * @brief Default constructor. Creates an empty/uninitialized buffer.
         */
        Buffer() = default;

        /**
         * @brief Creates a Vulkan buffer with the specified parameters.
         *
         * @param allocator VMA allocator used to create and manage the buffer.
         * @param size Size of the buffer in bytes.
         * @param usage Vulkan buffer usage flags.
         * @param memUsage VMA memory usage policy (e.g., GPU-only, CPU-to-GPU).
         * @param persistentlyMapped If true, the buffer will remain mapped for its entire lifetime.
         */
        Buffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
               VmaMemoryUsage memUsage, bool persistentlyMapped = false);

        /**
         * @brief Destructor. Frees the buffer and its associated memory.
         */
        ~Buffer();

        CP_NO_COPY_CLASS(Buffer);
        CP_HANDLE_CONVERSION(VkBuffer, m_buffer);

        /**
         * @brief Returns the underlying VMA allocation handle.
         * @return VmaAllocation Allocation handle.
         */
        VmaAllocation GetAllocation() const noexcept { return m_allocation; }

        /**
         * @brief Returns additional information about the VMA allocation.
         * @return const VmaAllocationInfo& Allocation metadata.
         */
        const VmaAllocationInfo &GetAllocationInfo() const noexcept { return m_allocationInfo; }

        void *GetMappedPtr() const noexcept { return m_mappedPtr; }

        /**
         * @brief Returns the Vulkan usage flags assigned to this buffer.
         * @return VkBufferUsageFlags Usage bitmask.
         */
        VkBufferUsageFlags GetUsage() const noexcept { return m_usage; }

        /**
         * @brief Returns the total size of the buffer in bytes.
         * @return VkDeviceSize Buffer size.
         */
        VkDeviceSize GetSize() const noexcept { return m_size; }

        /**
         * @brief Returns whether the buffer is persistently mapped.
         * @return true If the buffer remains mapped for its entire lifetime.
         * @return false Otherwise.
         */
        bool IsPersistentlyMapped() const noexcept { return m_persistentlyMapped; }

        /**
         * @brief Maps the buffer's memory if not already mapped.
         *
         * When persistent mapping is enabled, mapping occurs automatically
         * during construction.
         */
        void Map();

        /**
         * @brief Unmaps the buffer's memory if it was previously mapped.
         *
         * If the buffer is persistently mapped, this call is ignored.
         */
        void Unmap();

        /**
         * @brief Writes data into the buffer's mapped memory.
         *
         * @param src Pointer to the source memory.
         * @param sz Number of bytes to write.
         * @param offset Byte offset into the buffer where data should be copied.
         */
        void Write(const void *src, VkDeviceSize sz, VkDeviceSize offset = 0);

        /**
         * @brief Uploads data to the buffer using a temporary staging buffer.
         *
         * A staging buffer is created internally and freed upon transfer completion.
         *
         * @param device Logical device used to submit the upload.
         * @param cmdPool Command pool for allocating a transfer command buffer.
         * @param queue Queue used to submit the transfer operation.
         * @param srcData Pointer to the data to upload.
         * @param uploadSize Size of the data to upload.
         * @param wait If true, waits until the transfer is complete.
         * @return UploadHandle Handle representing the upload operation.
         */
        UploadHandle Upload(VkDevice device, VkCommandPool cmdPool, VkQueue queue,
                            const void *srcData, VkDeviceSize uploadSize, bool wait = true);

        /**
         * @brief Uploads data using a StagingRing for efficient batched transfers.
         *
         * @param ring Reference to the staging ring allocator.
         * @param device Logical Vulkan device.
         * @param cmdPool Command pool used for recording transfer commands.
         * @param queue Queue used to submit the copy.
         * @param srcData Pointer to the source data.
         * @param uploadSize Size of the data to upload.
         * @param align Required alignment for the ring buffer suballocation.
         * @param wait If true, waits for the transfer to finish.
         * @return UploadHandle Handle representing the upload operation.
         */
        UploadHandle UploadUsingRing(StagingRing &ring, VkDevice device, VkCommandPool cmdPool,
                                     VkQueue queue, const void *srcData, VkDeviceSize uploadSize,
                                     VkDeviceSize align = 16, bool wait = true);

        /**
         * @brief Resizes the buffer, reallocating memory and preserving usage flags.
         *
         * @param newSize New buffer size in bytes.
         * @param memUsage New VMA memory usage specification.
         */
        void Resize(VkDeviceSize newSize, VmaMemoryUsage memUsage);

    private:
        /**
         * @brief Internal helper to free the buffer and its allocation.
         */
        static void UploadAndFreeWhenComplete(UploadHandle handle);

        /**
         * @brief Returns the VMA allocator associated with a staging ring.
         * @param r StagingRing instance.
         * @return VmaAllocator Allocator used for ring buffers.
         */
        static VmaAllocator ringAllocator(StagingRing &r);

        /**
         * @brief Returns the VMA allocation from a staging ring entry.
         * @param r StagingRing instance.
         * @return VmaAllocation Allocation inside the ring.
         */
        static VmaAllocation ringAllocation(StagingRing &r);

        /**
         * @brief Frees all resources held by this buffer.
         */
        void destroy();

        /**
         * @brief Internal method that allocates a new buffer.
         *
         * @param createSize Size of the buffer to create.
         * @param createUsage Vulkan buffer usage flags.
         * @param memUsage Memory usage policy for VMA.
         */
        void createInternal(VkDeviceSize createSize, VkBufferUsageFlags createUsage, VmaMemoryUsage memUsage);

    private:
        VkBuffer m_buffer = VK_NULL_HANDLE;          ///< Vulkan buffer handle.
        VmaAllocator m_allocator = VK_NULL_HANDLE;   ///< VMA allocator used to create the buffer.
        VmaAllocation m_allocation = VK_NULL_HANDLE; ///< Memory allocation handle.
        VmaAllocationInfo m_allocationInfo{};        ///< Allocation metadata from VMA.
        VkBufferUsageFlags m_usage = 0;              ///< Buffer usage flags.
        VkDeviceSize m_size = 0;                     ///< Size of the buffer in bytes.

        bool m_persistentlyMapped = false; ///< Whether the buffer remains mapped at all times.
        void *m_mappedPtr = nullptr;       ///< Pointer to mapped CPU-visible memory.
    };
} // namespace cp::vulkan
