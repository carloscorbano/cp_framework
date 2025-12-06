#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/core/export.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"

namespace cp
{
    enum class WindowMode
    {
        Windowed,
        Borderless,
        Fullscreen
    };

    struct WindowInfo
    {
        int width;
        int height;
        const char *title;
        WindowMode mode;
        bool vsync;
    };

    class Window
    {
    public:
        Window(const WindowInfo &createInfo);
        ~Window();

        CP_RULE_OF_FIVE_DELETE(Window);

        GLFWwindow *GetWindowHandle() noexcept;

        const WindowMode &GetWindowMode() const;
        void SetWindowMode(const WindowMode &mode);

        void Update();
        bool ShouldClose() const;

        int GetWidth() const;
        int GetHeight() const;
        f32 GetAspectRatio() const;
        bool IsFocused() const;
        bool IsMinimized() const;
        bool IsVisible() const;

        void SetTitle(const std::string &title);
        void SetOpacity(f32 alpha);
        void SetAlwaysOnTop(bool enable);

        void SetClipboardText(const char *text);
        std::string GetClipboardText() const;

        void GetContentScale(f32 &x, f32 &y) const;

        bool IsVSyncEnabled() const;
        void SetVSyncEnabled(bool enabled);

    private:
        static void GLFW_WindowSizeCallback(GLFWwindow *window, int width, int height);
        static void GLFW_WindowPosCallback(GLFWwindow *window, int xpos, int ypos);
        static void GLFW_WindowFocusCallback(GLFWwindow *window, int focused);
        static void GLFW_WindowIconifyCallback(GLFWwindow *window, int iconified);
        static void GLFW_WindowMaximizeCallback(GLFWwindow *window, int maximized);
        static void GLFW_WindowCloseCallback(GLFWwindow *window);

        GLFWmonitor *getMonitorForWindow(GLFWwindow *window) const;
        void centerWindowOnScreen(GLFWwindow *window) const;
        void setWindowMode(const WindowMode &mode);

    private:
        GLFWwindow *m_wndHandle = nullptr;
        WindowInfo m_wndInfo;

        int m_prevX, m_prevY, m_prevW, m_prevH;
    };
} // namespace cp
