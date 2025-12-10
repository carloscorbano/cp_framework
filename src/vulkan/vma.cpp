#include "cp_framework/vulkan/vma.hpp"
#include "cp_framework/vulkan/physicalDevice.hpp"
#include "cp_framework/vulkan/instance.hpp"
#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/debug/debug.hpp"
#include "cp_framework/vulkan/types.hpp"

namespace cp::vulkan
{
    Vma::Vma(Instance &instance, Device &device, PhysicalDevice &physDevice)
    {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = physDevice.get();
        allocatorInfo.device = device.get();
        allocatorInfo.instance = instance.get();
        allocatorInfo.vulkanApiVersion = VK_API_VERSION;
        allocatorInfo.pAllocationCallbacks = nullptr;
        allocatorInfo.pDeviceMemoryCallbacks = nullptr;
        allocatorInfo.flags = 0;

        // Try to enable memory budget extension support if available in this build of VMA
#ifdef VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
#endif

        if (vmaCreateAllocator(&allocatorInfo, &m_vmaAlloc) != VK_SUCCESS)
        {
            LOG_THROW("Failed to create VMA allocator!");
        }

        LOG_INFO("VMA allocator created successfully");
    }

    Vma::~Vma()
    {
        CP_VK_DELETE_HANDLE(m_vmaAlloc, vmaDestroyAllocator(m_vmaAlloc));
    }
}