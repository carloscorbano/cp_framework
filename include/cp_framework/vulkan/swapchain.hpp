#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <vector>

namespace cp::vulkan
{
    class Instance;
    class Device;
    class PhysicalDevice;
    class Surface;
    class Swapchain
    {
    public:
        Swapchain(GLFWwindow *window, Instance &instance, Device &device, PhysicalDevice &physDevice, Surface &surface, VkPresentModeKHR preferredMode, Swapchain *oldSwapchain = nullptr);
        ~Swapchain();

        CP_RULE_OF_FIVE_DELETE(Swapchain);

        CP_HANDLE_CONVERSION(VkSwapchainKHR, m_swapchain);

        std::vector<VkImage> &GetImages() { return m_images; }
        std::vector<VkImageView> &GetViews() { return m_views; }
        VkFormat &GetColorFormat() { return m_colorFormat; }
        VkFormat &GetDepthFormat() { return m_depthFormat; }
        VkFormat &GetStencilFormat() { return m_stencilFormat; }
        VkExtent2D &GetExtent() { return m_extent; }

    private:
        Device &m_device;

        VkSwapchainKHR m_swapchain;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_views;
        VkFormat m_colorFormat;
        VkFormat m_depthFormat;
        VkFormat m_stencilFormat;
        VkExtent2D m_extent;
    };
} // namespace cp::vulkan
