#pragma once
#include "cp_framework/core/types.hpp"
#include "instance.hpp"
#include "surface.hpp"
#include "debugMessenger.hpp"
#include "physicalDevice.hpp"
#include "device.hpp"
#include "vma.hpp"
#include "swapchain.hpp"

#ifndef NDEBUG
constexpr bool validationLayersEnabled = true;
#else
constexpr bool validationLayersEnabled = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData);

namespace cp
{
    class VkManager
    {
    public:
        VkManager(GLFWwindow *window);
        ~VkManager();

        CP_NO_COPY_CLASS(VkManager);

        vulkan::Instance &GetInstance() { return m_instance; }
        vulkan::Surface &GetSurface() { return m_surface; }
        vulkan::PhysicalDevice &GetPhysicalDevice() { return m_physDevice; }
        vulkan::Device &GetDevice() { return m_device; }
        vulkan::Vma &GetVma() { return m_vma; }
        vulkan::Swapchain &GetSwapchain() { return m_swapchain; }

    private:
        std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        std::vector<const char *> requiredExtensions = {VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME};
        std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME};

        vulkan::Instance m_instance;
        vulkan::Surface m_surface;
        vulkan::DebugMessenger m_debugMessenger;
        vulkan::PhysicalDevice m_physDevice;
        vulkan::Device m_device;
        vulkan::Vma m_vma;
        vulkan::Swapchain m_swapchain;
    };
} // namespace cp
