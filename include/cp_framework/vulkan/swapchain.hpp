#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <vector>
#include <span>

namespace cp::vulkan
{
    /**
     * @brief Indicates the target layout used when transitioning a swapchain image.
     */
    enum class SwapchainImageLayoutTarget
    {
        TRANSFER,         ///< Image will be transitioned for use as a transfer destination.
        COLOR_ATTACHMENT, ///< Image will be transitioned for rendering as a color attachment.
        PRESENT           ///< Image will be transitioned for presentation to the screen.
    };

    class Instance;
    class Device;
    class PhysicalDevice;
    class Surface;

    /**
     * @brief Manages a Vulkan swapchain and all associated resources such as images, views and semaphores.
     *
     * This class handles swapchain creation, destruction, image acquisition and layout transitions.
     * The swapchain is automatically recreated on window resize.
     */
    class Swapchain
    {
    public:
        /**
         * @brief Constructs the swapchain and queries all necessary formats and capabilities.
         *
         * @param window Pointer to the GLFW window used for surface creation and size queries.
         * @param instance Vulkan instance the swapchain belongs to.
         * @param device Logical device used for swapchain operations.
         * @param physDevice Physical device providing surface and format capabilities.
         * @param surface Rendering surface tied to the OS window.
         * @param preferredMode Desired presentation mode (e.g. MAILBOX, FIFO).
         */
        Swapchain(GLFWwindow *window,
                  Instance &instance,
                  Device &device,
                  PhysicalDevice &physDevice,
                  Surface &surface,
                  VkPresentModeKHR preferredMode);

        /**
         * @brief Destroys the swapchain and all related Vulkan resources.
         */
        ~Swapchain();

        CP_RULE_OF_FIVE_DELETE(Swapchain);
        CP_HANDLE_CONVERSION(VkSwapchainKHR, m_swapchain);

        /**
         * @brief Returns the color format used by the swapchain images.
         */
        VkFormat &GetColorFormat() { return m_colorFormat; }

        /**
         * @brief Returns the chosen depth format used by the application.
         */
        VkFormat &GetDepthFormat() { return m_depthFormat; }

        /**
         * @brief Returns the selected stencil format.
         */
        VkFormat &GetStencilFormat() { return m_stencilFormat; }

        /**
         * @brief Returns the image extent (width and height) of swapchain images.
         */
        VkExtent2D &GetExtent() { return m_extent; }

        /**
         * @brief Recreates the swapchain, usually called on window resize events.
         *
         * @param preferredMode Preferred present mode when rebuilding the swapchain.
         */
        void Recreate(VkPresentModeKHR preferredMode);

        /**
         * @brief Acquires the next available swapchain image for rendering.
         *
         * @param availableSemaphore Semaphore signaled once the image is ready.
         * @param timeout Timeout in nanoseconds (defaults to UINT64_MAX for no timeout).
         * @return VkResult Result of image acquisition (e.g. VK_SUCCESS, VK_SUBOPTIMAL_KHR).
         */
        VkResult AcquireSwapchainNextImage(VkSemaphore availableSemaphore, uint64_t timeout = UINT64_MAX);

        /**
         * @brief Returns the VkImage of the currently acquired swapchain index.
         */
        VkImage GetCurrentImage() { return m_images[m_curImageIndex]; }

        /**
         * @brief Returns the VkImageView associated with the current swapchain image.
         */
        VkImageView GetCurrentImageView() { return m_views[m_curImageIndex]; }

        /**
         * @brief Returns the semaphore signaled when rendering is finished for the current image.
         */
        VkSemaphore GetCurrentRenderFinishedSemaphore() { return m_renderFinishedSemaphores[m_curImageIndex]; }

        /**
         * @brief Returns how many images the swapchain holds.
         */
        const size_t ImageCount() const { return m_images.size(); }

        /**
         * @brief Performs a pipeline barrier to transition the current swapchain image to the target layout.
         *
         * @param cmdBuffer Command buffer used for recording the barrier.
         * @param targetLayout Desired layout defined by SwapchainImageLayoutTarget.
         */
        void TransitionCurrentImageLayout(VkCommandBuffer cmdBuffer, const SwapchainImageLayoutTarget &targetLayout);

    private:
        /**
         * @brief Internal helper that creates a new swapchain instance.
         *
         * @param preferredMode Presentation mode requested by the caller.
         * @param oldSwapchain Optional handle to an old swapchain for efficient reuse.
         */
        void create(VkPresentModeKHR preferredMode, VkSwapchainKHR oldSwapchain);

        /**
         * @brief Destroys swapchain resources such as views and semaphores.
         *
         * @param swapchain Swapchain handle to clean up.
         * @param views Views associated with the swapchain images.
         * @param renderFinishedSemaphores Semaphores used for render completion.
         */
        void destroy(VkSwapchainKHR swapchain,
                     std::span<VkImageView> views,
                     std::span<VkSemaphore> renderFinishedSemaphores);

    private:
        GLFWwindow *m_window;         ///< GLFW window used to determine surface size.
        Instance &m_instance;         ///< Vulkan instance reference.
        Device &m_device;             ///< Logical device reference.
        PhysicalDevice &m_physDevice; ///< Physical device used for querying formats.
        Surface &m_surface;           ///< Rendering surface.

        VkSwapchainKHR m_swapchain; ///< Vulkan swapchain handle.

        std::vector<VkImage> m_images;    ///< Swapchain images.
        std::vector<VkImageView> m_views; ///< Image views for rendering.
        VkFormat m_colorFormat;           ///< Color buffer format.
        VkFormat m_depthFormat;           ///< Depth buffer format.
        VkFormat m_stencilFormat;         ///< Stencil buffer format.
        VkExtent2D m_extent;              ///< Dimensions of swapchain images.

        std::vector<VkSemaphore> m_renderFinishedSemaphores; ///< Signals that rendering is complete.
        uint32_t m_curImageIndex = 0;                        ///< Index of the currently acquired image.
    };
} // namespace cp::vulkan
