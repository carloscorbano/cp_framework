#include "cp_framework/vulkan/stagingRing.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    StagingRing::StagingRing(VmaAllocator allocator, VkDeviceSize totalSize, VkBufferUsageFlags usage)
    {
        if (allocator == VK_NULL_HANDLE)
            LOG_THROW("StagingRing: allocator null");

        create();
        startJanitor();
    }

    StagingRing::~StagingRing()
    {
        stopJanitor();
        destroy();
    }

    StagingRing::Reservation StagingRing::Reserve(VkDeviceSize size, VkDeviceSize align)
    {
        std::lock_guard<std::mutex> lk(commitMutex); // protect head updates
        if (size > totalSize)
            LOG_THROW("StagingRing::Reserve size > totalSize");
        // align head
        VkDeviceSize alignedHead = Align(head, align);

        if (alignedHead + size <= totalSize)
        {
            // fits until end
            Reservation r{alignedHead, static_cast<uint8_t *>(mapped) + alignedHead};
            head = alignedHead + size;
            return r;
        }
        else
        {
            // wrap to start
            alignedHead = Align(0, align);
            if (alignedHead + size > tail)
            {
                // no space (tail is occupied region not yet freed) -> we cannot alloc
                // Caller must ensure previous uploads finished or use larger ring.
                LOG_THROW("StagingRing::Reserve out of space - increase ring size or ensure GPU consumed previous uploads");
            }
            Reservation r{alignedHead, static_cast<uint8_t *>(mapped) + alignedHead};
            head = alignedHead + size;
            return r;
        }
    }

    void StagingRing::AdvanceTailTo(VkDeviceSize newTail)
    {
        std::lock_guard<std::mutex> lk(commitMutex);
        tail = newTail % totalSize;
    }

    VkDeviceSize StagingRing::Align(VkDeviceSize v, VkDeviceSize align)
    {
        return (v + (align - 1)) & ~(align - 1);
    }

    void StagingRing::SubmitAndTrack(UploadHandle handle, VkDeviceSize offset, VkDeviceSize size)
    {
        if (!handle.valid())
            return;
        Pending p;
        p.handle = handle;
        p.offsetEnd = (offset + size) % totalSize;
        {
            std::lock_guard<std::mutex> lk(queueMutex);
            pending.push(std::move(p));
        }
        cv.notify_one();
    }

    void StagingRing::create()
    {
        VkBufferCreateInfo bci{};
        bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bci.size = totalSize;
        bci.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo aci{};
        aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        aci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; // already mapped

        if (vmaCreateBuffer(allocator, &bci, &aci, &buffer, &allocation, &allocationInfo) != VK_SUCCESS)
        {
            LOG_THROW("StagingRing::create failed");
        }

        // vma provides pMappedData in allocationInfo when mapped
        mapped = allocationInfo.pMappedData;
        head = tail = 0;
    }

    void StagingRing::destroy()
    {
        if (buffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(allocator, buffer, allocation);
        }
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        allocationInfo = {};
        mapped = nullptr;
        totalSize = 0;
        head = tail = 0;
    }

    void StagingRing::startJanitor()
    {
        running.store(true);
        janitor = std::thread([this]()
                              { this->janitorLoop(); });
    }

    void StagingRing::stopJanitor()
    {
        running.store(false);
        cv.notify_one();
        if (janitor.joinable())
            janitor.join();
        // cleanup any remaining pending handles
        std::lock_guard<std::mutex> lk(queueMutex);
        while (!pending.empty())
        {
            auto p = pending.front();
            pending.pop();
            if (p.handle.valid())
            {
                if (p.handle.device && p.handle.fence)
                {
                    vkWaitForFences(p.handle.device, 1, &p.handle.fence, VK_TRUE, UINT64_MAX);
                    vkDestroyFence(p.handle.device, p.handle.fence, nullptr);
                }
                if (p.handle.device && p.handle.cmd && p.handle.pool)
                {
                    vkFreeCommandBuffers(p.handle.device, p.handle.pool, 1, &p.handle.cmd);
                }
            }
        }
    }

    void StagingRing::janitorLoop()
    {
        while (running.load())
        {
            Pending item;
            {
                std::unique_lock<std::mutex> lk(queueMutex);
                cv.wait(lk, [this]
                        { return !pending.empty() || !running.load(); });
                if (!running.load() && pending.empty())
                    break;
                item = pending.front();
                pending.pop();
            }

            if (!item.handle.valid())
                continue;
            // wait for fence (block here per item) then cleanup and advance tail
            if (item.handle.device && item.handle.fence)
            {
                vkWaitForFences(item.handle.device, 1, &item.handle.fence, VK_TRUE, UINT64_MAX);
                vkDestroyFence(item.handle.device, item.handle.fence, nullptr);
            }
            if (item.handle.device && item.handle.cmd && item.handle.pool)
            {
                vkFreeCommandBuffers(item.handle.device, item.handle.pool, 1, &item.handle.cmd);
            }

            // advance tail to offsetEnd
            {
                std::lock_guard<std::mutex> lk(commitMutex);
                tail = item.offsetEnd;
            }
        }
    }
}