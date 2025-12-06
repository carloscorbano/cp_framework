#include "cp_framework/graphics/window.hpp"
#include "cp_framework/core/debug.hpp"

namespace cp
{
    Window::Window(const WindowInfo &createInfo)
        : m_wndInfo(createInfo)
    {
        if (!glfwInit())
        {
            LOG_THROW("[WINDOW] Failed to initialize GLFW!");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (!glfwVulkanSupported())
        {
            LOG_THROW("[WINDOW] Vulkan is not supported!");
        }

        m_wndHandle = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, 0, 0);
        if (!m_wndHandle)
        {
            LOG_THROW("[WINDOW] Failed to create glfw window!");
        }

        glfwSetWindowUserPointer(m_wndHandle, this);
        glfwSetWindowSizeCallback(m_wndHandle, GLFW_WindowSizeCallback);
        glfwSetWindowPosCallback(m_wndHandle, GLFW_WindowPosCallback);
        glfwSetWindowFocusCallback(m_wndHandle, GLFW_WindowFocusCallback);
        glfwSetWindowIconifyCallback(m_wndHandle, GLFW_WindowIconifyCallback);
        glfwSetWindowMaximizeCallback(m_wndHandle, GLFW_WindowMaximizeCallback);
        glfwSetWindowCloseCallback(m_wndHandle, GLFW_WindowCloseCallback);

        centerWindowOnScreen(m_wndHandle);
        setWindowMode(createInfo.mode);
    }

    Window::~Window()
    {
        if (m_wndHandle)
        {
            glfwDestroyWindow(m_wndHandle);
        }

        glfwTerminate();
    }

    GLFWwindow *Window::GetWindowHandle() noexcept
    {
        return m_wndHandle;
    }

    const WindowMode &Window::GetWindowMode() const
    {
        return m_wndInfo.mode;
    }

    void Window::SetWindowMode(const WindowMode &mode)
    {
        if (mode == GetWindowMode())
            return;

        setWindowMode(mode);
    }

    void Window::Update()
    {
        glfwPollEvents();
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(m_wndHandle);
    }

    int Window::GetWidth() const
    {
        return m_wndInfo.width;
    }

    int Window::GetHeight() const
    {
        return m_wndInfo.height;
    }

    f32 Window::GetAspectRatio() const
    {
        int w = GetWidth(), h = GetHeight();
        return h != 0 ? float(w) / h : 1.f;
    }

    bool Window::IsFocused() const
    {
        return glfwGetWindowAttrib(m_wndHandle, GLFW_FOCUSED) == GLFW_TRUE;
    }

    bool Window::IsMinimized() const
    {
        return glfwGetWindowAttrib(m_wndHandle, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    bool Window::IsVisible() const
    {
        return glfwGetWindowAttrib(m_wndHandle, GLFW_VISIBLE) == GLFW_TRUE;
    }

    void Window::SetTitle(const std::string &title)
    {
        glfwSetWindowTitle(m_wndHandle, title.c_str());
    }

    void Window::SetOpacity(f32 alpha)
    {
        glfwSetWindowOpacity(m_wndHandle, alpha);
    }

    void Window::SetAlwaysOnTop(bool enable)
    {
        glfwSetWindowAttrib(m_wndHandle, GLFW_FLOATING, enable ? GLFW_TRUE : GLFW_FALSE);
    }

    void Window::SetClipboardText(const char *text)
    {
        glfwSetClipboardString(m_wndHandle, text);
    }

    std::string Window::GetClipboardText() const
    {
        const char *txt = glfwGetClipboardString(m_wndHandle);
        return txt ? txt : "";
    }

    void Window::GetContentScale(f32 &x, f32 &y) const
    {
        glfwGetWindowContentScale(m_wndHandle, &x, &y);
    }

    bool Window::IsVSyncEnabled() const
    {
        return m_wndInfo.vsync;
    }

    void Window::SetVSyncEnabled(bool enabled)
    {
        m_wndInfo.vsync = enabled;
    }

    void Window::GLFW_WindowSizeCallback(GLFWwindow *window, int width, int height)
    {
    }

    void Window::GLFW_WindowPosCallback(GLFWwindow *window, int xpos, int ypos)
    {
    }

    void Window::GLFW_WindowFocusCallback(GLFWwindow *window, int focused)
    {
    }

    void Window::GLFW_WindowIconifyCallback(GLFWwindow *window, int iconified)
    {
    }

    void Window::GLFW_WindowMaximizeCallback(GLFWwindow *window, int maximized)
    {
    }

    void Window::GLFW_WindowCloseCallback(GLFWwindow *window)
    {
    }

    GLFWmonitor *Window::getMonitorForWindow(GLFWwindow *window) const
    {
        int wx, wy, ww, wh;
        glfwGetWindowPos(window, &wx, &wy);
        glfwGetWindowSize(window, &ww, &wh);

        int count;
        GLFWmonitor **monitors = glfwGetMonitors(&count);
        if (!monitors || count == 0)
            return glfwGetPrimaryMonitor();

        GLFWmonitor *bestMonitor = nullptr;
        int bestOverlap = 0;

        for (int i = 0; i < count; ++i)
        {
            int mx, my;
            glfwGetMonitorPos(monitors[i], &mx, &my);
            const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
            int mw = mode->width;
            int mh = mode->height;

            int overlap =
                std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
                std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

            if (overlap > bestOverlap)
            {
                bestOverlap = overlap;
                bestMonitor = monitors[i];
            }
        }

        return bestMonitor ? bestMonitor : glfwGetPrimaryMonitor();
    }

    void Window::centerWindowOnScreen(GLFWwindow *window) const
    {
        GLFWmonitor *monitor = getMonitorForWindow(window);
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);

        int mx, my;
        glfwGetMonitorPos(monitor, &mx, &my);

        int w, h;
        glfwGetWindowSize(window, &w, &h);

        int xpos = mx + (vidmode->width - w) / 2;
        int ypos = my + (vidmode->height - h) / 2;

        glfwSetWindowPos(window, xpos, ypos);
    }

    void Window::setWindowMode(const WindowMode &mode)
    {
        GLFWmonitor *monitor = getMonitorForWindow(m_wndHandle);
        if (!monitor)
            monitor = glfwGetPrimaryMonitor();

        const GLFWvidmode *vidMode = glfwGetVideoMode(monitor);

        int mx, my;
        glfwGetMonitorPos(monitor, &mx, &my);

        if (mode == WindowMode::Windowed)
        {
            glfwGetWindowPos(m_wndHandle, &m_prevX, &m_prevY);
            glfwGetWindowSize(m_wndHandle, &m_prevW, &m_prevH);
        }

        switch (mode)
        {
        case WindowMode::Windowed:
            glfwSetWindowAttrib(m_wndHandle, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowMonitor(m_wndHandle, nullptr, m_prevX, m_prevY, m_prevW, m_prevH, 0);
            break;
        case WindowMode::Borderless:
            glfwSetWindowAttrib(m_wndHandle, GLFW_DECORATED, GLFW_FALSE);
            glfwSetWindowMonitor(m_wndHandle, nullptr, mx, my, vidMode->width, vidMode->height, 0);
            break;
        case WindowMode::Fullscreen:
            glfwSetWindowMonitor(m_wndHandle, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
            break;
        }

        m_wndInfo.mode = mode;
    }

} // namespace cp
