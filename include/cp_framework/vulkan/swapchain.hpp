#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <vector>
#include <span>

namespace cp::vulkan
{
    class Instance;
    class Device;
    class PhysicalDevice;
    class Surface;
    class Swapchain
    {
    public:
        Swapchain(GLFWwindow *window, Instance &instance, Device &device, PhysicalDevice &physDevice, Surface &surface, VkPresentModeKHR preferredMode);
        ~Swapchain();

        CP_RULE_OF_FIVE_DELETE(Swapchain);

        CP_HANDLE_CONVERSION(VkSwapchainKHR, m_swapchain);

        std::span<const VkImage> GetImages() { return m_images; }
        std::span<const VkImageView> GetViews() { return m_views; }
        VkFormat &GetColorFormat() { return m_colorFormat; }
        VkFormat &GetDepthFormat() { return m_depthFormat; }
        VkFormat &GetStencilFormat() { return m_stencilFormat; }
        VkExtent2D &GetExtent() { return m_extent; }

        void Recreate(VkPresentModeKHR preferredMode);
        VkResult AcquireSwapchainNextImage(VkSemaphore availableSemaphore, uint32_t *outIndex, uint64_t timeout = UINT64_MAX);

        const size_t ImageCount() const { return m_images.size(); }

    private:
        void create(VkPresentModeKHR preferredMode, VkSwapchainKHR oldSwapchain);
        void destroy(VkSwapchainKHR swapchain, std::span<VkImageView> views, std::span<VkSemaphore> renderFinishedSemaphores);

    private:
        GLFWwindow *m_window;
        Instance &m_instance;
        Device &m_device;
        PhysicalDevice &m_physDevice;
        Surface &m_surface;

        VkSwapchainKHR m_swapchain;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_views;
        VkFormat m_colorFormat;
        VkFormat m_depthFormat;
        VkFormat m_stencilFormat;
        VkExtent2D m_extent;

        std::vector<VkSemaphore> m_swapchainImageSemaphore;
    };
} // namespace cp::vulkan
