#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include "cp_framework/thirdparty/vma/vma.inc.hpp"

namespace cp::vulkan
{
    class Device;
    class Vma;

    /**
     * @brief Wrapper class for a Vulkan image with VMA allocation and utility helpers.
     *
     * This class handles creation, memory allocation, layout transitions, and
     * copy operations for a Vulkan image. It also manages the corresponding
     * VkImageView and tracks image state such as layout and extent.
     */
    class Image
    {
    public:
        /**
         * @brief Constructs a Vulkan image with the given parameters.
         *
         * @param device Reference to the Vulkan device wrapper.
         * @param vma Reference to the VMA allocator wrapper.
         * @param width Image width in pixels.
         * @param height Image height in pixels.
         * @param format Vulkan image format.
         * @param usage Image usage flags (e.g., transfer, sampling, attachment).
         * @param memoryUsage VMA memory usage type (e.g., GPU-only, CPU-to-GPU).
         * @param aspectMask Aspect mask used for the VkImageView (e.g., color, depth).
         */
        Image(Device &device, Vma &vma, uint32_t width, uint32_t height, VkFormat format,
              VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageAspectFlags aspectMask);

        Image(Device &device, Vma &vma, void *data, VkCommandBuffer cmdBuffer, uint32_t width, uint32_t height, VkFormat format,
              VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageAspectFlags aspectMask);

        /**
         * @brief Destructor that automatically frees the Vulkan image and its resources.
         */
        ~Image();

        CP_NO_COPY_CLASS(Image);
        CP_HANDLE_CONVERSION(VkImage, m_image);

        /**
         * @brief Returns the image view associated with this image.
         * @return VkImageView The Vulkan image view.
         */
        inline VkImageView GetView() const noexcept { return m_view; }

        /**
         * @brief Returns the VMA memory allocation handle.
         * @return VmaAllocation Allocation handle.
         */
        inline VmaAllocation GetAllocation() const noexcept { return m_allocation; }

        /**
         * @brief Returns additional information about the VMA allocation.
         * @return const VmaAllocationInfo& Allocation metadata.
         */
        inline const VmaAllocationInfo &GetAllocationInfo() const noexcept { return m_allocationInfo; }

        /**
         * @brief Returns the current Vulkan image layout.
         * @return VkImageLayout The image layout.
         */
        inline VkImageLayout GetLayout() const noexcept { return m_layout; }

        /**
         * @brief Returns the Vulkan format of the image.
         * @return VkFormat The image format.
         */
        inline VkFormat GetFormat() const noexcept { return m_format; }

        /**
         * @brief Returns the 3D extent of the image (width, height, depth).
         * @return const VkExtent3D& The image extent.
         */
        inline const VkExtent3D &GetExtent() const noexcept { return m_extent; }

        /**
         * @brief Returns the usage flags the image was created with.
         * @return VkImageUsageFlags Image usage bitmask.
         */
        inline VkImageUsageFlags GetUsage() const noexcept { return m_usage; }

        /**
         * @brief Transitions the image to a new layout using a command buffer.
         *
         * Inserts a pipeline barrier to update the image's VkImageLayout.
         *
         * @param cmdBuffer Command buffer used to record the barrier.
         * @param newLayout The new layout to transition to.
         */
        void TransitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);

        /**
         * @brief Copies the content from another image into this image.
         *
         * Both images must have compatible formats and layouts suitable
         * for transfer operations.
         *
         * @param cmdBuffer Command buffer used for issuing the copy.
         * @param other Source image.
         * @param width Width of the region to copy.
         * @param height Height of the region to copy.
         * @param mipLevel Mipmap level to copy.
         * @param layerCount Number of array layers to copy.
         */
        void CopyFromImage(VkCommandBuffer cmdBuffer, const Image &other, uint32_t width, uint32_t height,
                           uint32_t mipLevel, uint32_t layerCount);

        void CopyFromCPU(VkCommandBuffer cmdBuffer, const void *data, uint32_t width, uint32_t height, uint32_t channels = 4);

    private:
        void createInternal(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageAspectFlags aspectMask);

        /**
         * @brief Internal cleanup method used by the destructor.
         */
        void destroy();

    private:
        VkImage m_image = VK_NULL_HANDLE;                   ///< Vulkan image handle.
        VkImageView m_view = VK_NULL_HANDLE;                ///< Image view used for pipeline binding.
        VmaAllocation m_allocation = VK_NULL_HANDLE;        ///< VMA allocation handle.
        VmaAllocationInfo m_allocationInfo{};               ///< Optional allocation metadata.
        VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED; ///< Current image layout.
        VkFormat m_format = VK_FORMAT_UNDEFINED;            ///< Image format.
        VkExtent3D m_extent{};                              ///< Image extent (width, height, depth).
        uint32_t m_channels;                                ///< Number of color channels.
        VkImageUsageFlags m_usage = 0;                      ///< Usage flags for reference/debugging.

        Device &m_device; ///< Reference to the Vulkan device wrapper.
        Vma &m_vma;       ///< Reference to the VMA allocator wrapper.
    };
} // namespace cp::vulkan
