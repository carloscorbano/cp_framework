#pragma once
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include "cp_framework/core/types.hpp"

namespace cp::vulkan
{
    class Instance;
    class Surface
    {
    public:
        Surface(GLFWwindow *window, Instance &instance);
        ~Surface();

        CP_RULE_OF_FIVE_DELETE(Surface);

        CP_HANDLE_CONVERSION(VkSurfaceKHR, m_surface);

    private:
        Instance &m_instance;
        VkSurfaceKHR m_surface;
    };
} // namespace cp::vulkan
