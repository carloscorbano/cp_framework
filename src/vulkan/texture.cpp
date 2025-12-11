#include "cp_framework/vulkan/texture.hpp"
#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/vulkan/vma.hpp"
#include "cp_framework/vulkan/image.hpp"
#include "cp_framework/thirdparty/stb/stb.inc.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    Texture::Texture(Device &device, Vma &vma, VkCommandBuffer cmd, std::span<const uint8_t> data, VkImageAspectFlags aspectFlags)
    {
        int width, height, channels;
        if (!stbi_info_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, &channels))
        {
            LOG_THROW("Failed to get image info from memory for texture creation.");
        }

        VkFormat format = VK_FORMAT_UNDEFINED;
        switch (channels)
        {
        case 1:
            VK_FORMAT_R8_UNORM;
            break;
        case 2:
            format = VK_FORMAT_R8G8_UNORM;
            break;
        case 3:
            format = VK_FORMAT_R8G8B8_UNORM;
            break;
        case 4:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            format = VK_FORMAT_UNDEFINED;
            break;
        }

        if (format == VK_FORMAT_UNDEFINED)
        {
            LOG_THROW("Unsupported number of channels in texture image: {}", channels);
        }

        m_image = M_UPTR<Image>(device, vma, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO, aspectFlags);

        auto img = stbi_load_from_memory(data.data(), static_cast<int>(data.size()), reinterpret_cast<int *>(&width),
                                         reinterpret_cast<int *>(&height), reinterpret_cast<int *>(&channels), STBI_rgb_alpha);

        if (!img)
        {
            LOG_THROW("Failed to load image from memory for texture creation.");
        }

        m_image->CopyFromCPU(cmd, img, width, height);

        stbi_image_free(img);
    }

    Texture::~Texture()
    {
        // image will be destroyed at destruction.
    }
}