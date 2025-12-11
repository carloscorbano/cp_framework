#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <memory>
#include <span>

namespace cp::vulkan
{
    class Image;
    class Device;
    class Vma;
    class Texture
    {
    public:
        Texture(Device &device, Vma &vma, VkCommandBuffer cmd, std::span<const uint8_t> data, VkImageAspectFlags aspectFlags);
        ~Texture();

        CP_NO_COPY_CLASS(Texture);

    private:
        std::unique_ptr<Image> m_image;
    };
}