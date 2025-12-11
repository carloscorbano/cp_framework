#include "cp_framework/vulkan/image.hpp"
#include "cp_framework/debug/debug.hpp"
#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/vulkan/vma.hpp"
#include "cp_framework/vulkan/utils.hpp"
#include "cp_framework/vulkan/buffer.hpp"

namespace cp::vulkan
{
    Image::Image(Device &device, Vma &vma, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageAspectFlags aspectMask)
        : m_device(device), m_vma(vma)
    {
        createInternal(width, height, format, usage, memoryUsage, aspectMask);
    }

    Image::Image(Device &device, Vma &vma, void *data, VkCommandBuffer cmdBuffer, uint32_t width, uint32_t height, VkFormat format,
                 VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageAspectFlags aspectMask)
        : m_device(device), m_vma(vma)
    {
        createInternal(width, height, format, usage, memoryUsage, aspectMask);
        CopyFromCPU(cmdBuffer, data, width, height);
    }

    Image::~Image()
    {
        destroy();
    }

    void Image::TransitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
    {
        if (m_layout == newLayout)
            return;

        utils::TransitionImageLayout(cmdBuffer, m_image, m_format, m_layout, newLayout);
        m_layout = newLayout;
    }

    void Image::CopyFromImage(VkCommandBuffer cmdBuffer, const Image &other, uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t layerCount)
    {
        if (this == &other || other.m_image == VK_NULL_HANDLE || m_image == VK_NULL_HANDLE || width == 0 || height == 0 || layerCount == 0 || width > m_extent.width || height > m_extent.height)
            return;

        utils::CopyImage(cmdBuffer, other.m_image, m_image, width, height, mipLevel, layerCount);
    }

    void Image::CopyFromCPU(VkCommandBuffer cmdBuffer, const void *data, uint32_t width, uint32_t height, uint32_t channels)
    {
        if (m_image == VK_NULL_HANDLE || width == 0 || height == 0 || width > m_extent.width || height > m_extent.height)
            return;

        // Staging buffer:
        Buffer stagingBuffer(m_vma.get(), width * height * channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, true);
        void *mappedPtr = stagingBuffer.GetMappedPtr();

        // do the image transition layout
        utils::TransitionImageLayout(cmdBuffer, m_image, m_format, m_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // copy to staging
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.imageExtent = {width, height, 1};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;

        vkCmdCopyBufferToImage(
            cmdBuffer,
            stagingBuffer.get(),
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);

        // Final layout
        utils::TransitionImageLayout(cmdBuffer, m_image, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // staging will be erase at destruction
    }

    void Image::destroy()
    {
        CP_VK_DESTROY(m_device.get(), m_view, vkDestroyImageView);
        vmaDestroyImage(m_vma.get(), m_image, m_allocation);
        m_image = VK_NULL_HANDLE;
    }

    void Image::createInternal(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageAspectFlags aspectMask)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;

        if (vmaCreateImage(m_vma.get(), &imageInfo, &allocInfo, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
        {
            LOG_THROW("Failed to create VMA image");
        }

        vmaGetAllocationInfo(m_vma.get(), m_allocation, &m_allocationInfo);

        m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_format = imageInfo.format;
        m_usage = imageInfo.usage;
        m_extent = imageInfo.extent;

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectMask;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device.get(), &viewInfo, nullptr, &m_view) != VK_SUCCESS)
        {
            vmaDestroyImage(m_vma.get(), m_image, m_allocation);
            m_image = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
            LOG_THROW("Failed to create image view!");
        }
    }
}