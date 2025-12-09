#include "cp_framework/vulkan/buffer.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    Buffer::Buffer(VmaAllocator allocator, VkDeviceSize m_size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage, bool persistentlyMapped)
        : m_allocator(allocator),
          m_usage(usage),
          m_size(m_size),
          m_persistentlyMapped(persistentlyMapped)
    {
        createInternal(m_size, m_usage, memUsage);
        if (m_persistentlyMapped)
        {
            if (m_allocationInfo.pMappedData)
                m_mappedPtr = m_allocationInfo.pMappedData;
            else
            {
                // try explicit map
                void *p = nullptr;
                if (vmaMapMemory(m_allocator, m_allocation, &p) == VK_SUCCESS)
                    m_mappedPtr = p;
                else
                {
                    destroy();
                    LOG_THROW("Requested m_persistentlyMapped but m_allocation isn't mappable");
                }
            }
        }
    }

    Buffer::~Buffer()
    {
        destroy();
    }

    void Buffer::Map()
    {
        if (m_mappedPtr)
            return;
        if (m_allocation == VK_NULL_HANDLE)
            throw std::runtime_error("VulkanBuffer::Map: no m_allocation");
        if (vmaMapMemory(m_allocator, m_allocation, &m_mappedPtr) != VK_SUCCESS)
        {
            LOG_THROW("VulkanBuffer::Map failed");
        }
    }

    void Buffer::Unmap()
    {
        if (!m_mappedPtr)
            return;
        vmaUnmapMemory(m_allocator, m_allocation);
        m_mappedPtr = nullptr;
    }

    void Buffer::Write(const void *src, VkDeviceSize sz, VkDeviceSize offset)
    {
        if (!src)
            throw std::invalid_argument("VulkanBuffer::Write: src is null");
        if (offset + sz > m_size)
            throw std::out_of_range("VulkanBuffer::Write: write exceeds m_buffer m_size");
        if (m_allocation == VK_NULL_HANDLE)
            throw std::runtime_error("VulkanBuffer::Write: m_buffer not allocated");

        if (m_mappedPtr)
        {
            std::memcpy(static_cast<uint8_t *>(m_mappedPtr) + offset, src, static_cast<size_t>(sz));
            vmaFlushAllocation(m_allocator, m_allocation, offset, sz);
        }
        else
        {
            void *p = nullptr;
            if (vmaMapMemory(m_allocator, m_allocation, &p) != VK_SUCCESS)
                LOG_THROW("VulkanBuffer::Write map failed");
            std::memcpy(static_cast<uint8_t *>(p) + offset, src, static_cast<size_t>(sz));
            vmaFlushAllocation(m_allocator, m_allocation, offset, sz);
            vmaUnmapMemory(m_allocator, m_allocation);
        }
    }

    UploadHandle Buffer::Upload(VkDevice device, VkCommandPool cmdPool, VkQueue queue, const void *srcData, VkDeviceSize uploadSize, bool wait)
    {
        if (!srcData)
            throw std::invalid_argument("VulkanBuffer::Upload: srcData null");
        if (uploadSize == 0 || uploadSize > m_size)
            throw std::invalid_argument("VulkanBuffer::Upload: bad m_size");
        if (m_buffer == VK_NULL_HANDLE)
            throw std::runtime_error("VulkanBuffer::Upload: destination m_buffer not created");

        // Create staging temp (mapped)
        Buffer staging(m_allocator, uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, true);
        staging.Write(srcData, uploadSize, 0);

        // Allocate one-time command m_buffer
        VkCommandBufferAllocateInfo cbAlloc{};
        cbAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbAlloc.commandPool = cmdPool;
        cbAlloc.commandBufferCount = 1;

        VkCommandBuffer cmd{};
        if (vkAllocateCommandBuffers(device, &cbAlloc, &cmd) != VK_SUCCESS)
        {
            LOG_THROW("VulkanBuffer::Upload failed to allocate command m_buffer");
        }

        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &begin);

        VkBufferCopy region{};
        region.srcOffset = 0;
        region.dstOffset = 0;
        region.size = uploadSize;
        vkCmdCopyBuffer(cmd, staging.get(), this->m_buffer, 1, &region);

        vkEndCommandBuffer(cmd);

        VkFenceCreateInfo fci{};
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        if (vkCreateFence(device, &fci, nullptr, &fence) != VK_SUCCESS)
        {
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
            LOG_THROW("VulkanBuffer::Upload failed to create fence");
        }

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        if (vkQueueSubmit(queue, 1, &submit, fence) != VK_SUCCESS)
        {
            vkDestroyFence(device, fence, nullptr);
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
            LOG_THROW("VulkanBuffer::Upload failed to submit");
        }

        UploadHandle handle;
        handle.fence = fence;
        handle.cmd = cmd;
        handle.device = device;
        handle.pool = cmdPool;

        if (wait)
        {
            vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
            vkDestroyFence(device, fence, nullptr);
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
            return UploadHandle{};
        }
        else
        {
            return handle;
        }
    }

    UploadHandle Buffer::UploadUsingRing(StagingRing &ring, VkDevice device, VkCommandPool cmdPool, VkQueue queue, const void *srcData, VkDeviceSize uploadSize, VkDeviceSize align, bool wait)
    {
        if (!srcData)
            throw std::invalid_argument("VulkanBuffer::UploadUsingRing: srcData null");
        if (uploadSize == 0 || uploadSize > m_size)
            throw std::invalid_argument("VulkanBuffer::UploadUsingRing: bad size");
        if (ring.GetBuffer() == VK_NULL_HANDLE)
            LOG_THROW("Staging ring not created");

        // Reserve space in ring
        auto res = ring.Reserve(uploadSize, align);
        // copy into mapped staging ring
        std::memcpy(res.ptr, srcData, static_cast<size_t>(uploadSize));
        // flush m_allocation region (the ring uses VMA CPU_TO_GPU mapped)
        vmaFlushAllocation(ringAllocator(ring), ringAllocation(ring), res.offset, uploadSize); // helper functions below

        // create command m_buffer
        VkCommandBufferAllocateInfo cbAlloc{};
        cbAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbAlloc.commandPool = cmdPool;
        cbAlloc.commandBufferCount = 1;

        VkCommandBuffer cmd{};
        if (vkAllocateCommandBuffers(device, &cbAlloc, &cmd) != VK_SUCCESS)
        {
            LOG_THROW("VulkanBuffer::UploadUsingRing failed to allocate command m_buffer");
        }

        VkCommandBufferBeginInfo bbi{};
        bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &bbi);

        VkBufferCopy region{};
        region.srcOffset = res.offset;
        region.dstOffset = 0;
        region.size = uploadSize;
        vkCmdCopyBuffer(cmd, ring.GetBuffer(), this->m_buffer, 1, &region);

        vkEndCommandBuffer(cmd);

        VkFenceCreateInfo fci{};
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        if (vkCreateFence(device, &fci, nullptr, &fence) != VK_SUCCESS)
        {
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
            LOG_THROW("VulkanBuffer::UploadUsingRing failed to create fence");
        }

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        if (vkQueueSubmit(queue, 1, &submit, fence) != VK_SUCCESS)
        {
            vkDestroyFence(device, fence, nullptr);
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
            LOG_THROW("VulkanBuffer::UploadUsingRing failed to submit");
        }

        // Advance tail is left to user — but we can optionally compute minimal consumption later.
        // For simplicity, we do NOT change ring.tail here: user may call ring.AdvanceTailTo(...) with appropriate value
        // after waiting for fence. Alternatively use UploadAndFreeWhenComplete(handle) to advance tail automatically.

        UploadHandle handle;
        handle.fence = fence;
        handle.cmd = cmd;
        handle.device = device;
        handle.pool = cmdPool;

        if (wait)
        {
            vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
            vkDestroyFence(device, fence, nullptr);
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
            // Optionally advance ring.tail fully here (not done because we don't know which offset consumed previously)
            return UploadHandle{};
        }
        else
        {
            // Track this upload with the ring so the ring janitor will advance tail when the fence completes
            ring.SubmitAndTrack(handle, res.offset, uploadSize);
            return handle;
        }
    }

    void Buffer::UploadAndFreeWhenComplete(UploadHandle handle)
    {
        if (!handle.valid())
            return;
        // detach a thread that waits and then cleans resources
        std::thread([h = handle]() mutable
                    {
                if (h.device == VK_NULL_HANDLE) return;
                vkWaitForFences(h.device, 1, &h.fence, VK_TRUE, UINT64_MAX);
                vkDestroyFence(h.device, h.fence, nullptr);
                if (h.cmd != VK_NULL_HANDLE && h.pool != VK_NULL_HANDLE) {
                    vkFreeCommandBuffers(h.device, h.pool, 1, &h.cmd);
                } })
            .detach();
    }

    VmaAllocator Buffer::ringAllocator(StagingRing &r)
    {
        return r.getAllocator();
    }

    VmaAllocation Buffer::ringAllocation(StagingRing &r)
    {
        return r.getAllocation();
    }

    void Buffer::Resize(VkDeviceSize newSize, VmaMemoryUsage memUsage)
    {
        if (newSize == m_size)
            return;
        destroy();
        createInternal(newSize, m_usage, memUsage);
        if (m_persistentlyMapped)
        {
            if (vmaMapMemory(m_allocator, m_allocation, &m_mappedPtr) != VK_SUCCESS)
            {
                LOG_THROW("VulkanBuffer::Resize: failed to map new m_allocation");
            }
        }
    }

    void Buffer::destroy()
    {
        if (m_mappedPtr && m_allocation != VK_NULL_HANDLE && !m_persistentlyMapped)
        {
            vmaUnmapMemory(m_allocator, m_allocation);
            m_mappedPtr = nullptr;
        }

        if (m_buffer != VK_NULL_HANDLE)
        {
            LOG_INFO("VulkanBuffer::destroy m_size {}", m_allocationInfo.size);
            vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
        }

        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
        m_allocationInfo = {};
        m_usage = 0;
        m_size = 0;
        m_allocator = VK_NULL_HANDLE;
        m_persistentlyMapped = false;
        m_mappedPtr = nullptr;
    }

    void Buffer::createInternal(VkDeviceSize createSize, VkBufferUsageFlags createUsage, VmaMemoryUsage memUsage)
    {
        if (m_allocator == VK_NULL_HANDLE)
            LOG_THROW("VulkanBuffer: m_allocator is null");

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = createSize;
        bufferInfo.usage = createUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memUsage;
        allocInfo.flags = 0;

        if (m_persistentlyMapped)
        {
            allocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        this->m_allocator = m_allocator;
        this->m_usage = createUsage;
        this->m_size = createSize;

        if (vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, &m_allocationInfo) != VK_SUCCESS)
        {
            LOG_THROW("VulkanBuffer::createInternal failed to create m_buffer via VMA");
        }

        LOG_INFO("VulkanBuffer::created m_size {}, is persistantly mapped? {}", m_allocationInfo.size, m_persistentlyMapped);

        if (m_persistentlyMapped && m_allocationInfo.pMappedData)
        {
            m_mappedPtr = m_allocationInfo.pMappedData;
        }
    }
}