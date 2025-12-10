#include "cp_framework/vulkan/surface.hpp"
#include "cp_framework/vulkan/instance.hpp"
#include "cp_framework/debug/debug.hpp"

namespace cp::vulkan
{
    Surface::Surface(GLFWwindow *window, Instance &instance)
        : m_instance(instance)
    {
        if (glfwCreateWindowSurface(instance.get(), window, nullptr, &m_surface) != VK_SUCCESS)
        {
            LOG_THROW("[VULKAN] Failed to create window surface!");
        }
    }

    Surface::~Surface()
    {
        CP_VK_DESTROY(m_instance.get(), m_surface, vkDestroySurfaceKHR);
    }
} // namespace cp::vulkan
