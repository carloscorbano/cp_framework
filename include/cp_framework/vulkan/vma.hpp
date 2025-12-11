#pragma once

#include "cp_framework/thirdparty/vma/vma.inc.hpp"
#include "cp_framework/core/types.hpp"

namespace cp::vulkan
{
    class Instance;
    class Device;
    class PhysicalDevice;
    class Vma
    {
    public:
        Vma(Instance &instance, Device &device, PhysicalDevice &physDevice);
        ~Vma();

        CP_NO_COPY_CLASS(Vma);

        CP_HANDLE_CONVERSION(VmaAllocator, m_vmaAlloc);

    private:
        VmaAllocator m_vmaAlloc;
    };
} // namespace cp::vulkan
