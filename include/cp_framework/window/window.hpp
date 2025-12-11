#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/core/export.hpp"
#include "cp_framework/thirdparty/glfw/glfw.inc.hpp"
#include <atomic>
#include <chrono>

namespace cp
{
    /**
     * @brief Defines the different modes a window can be in.
     */
    enum class WindowMode
    {
        Windowed,   ///< Standard window with borders and title bar
        Borderless, ///< Window without borders (usually covers the screen)
        Fullscreen  ///< Exclusive fullscreen mode on a monitor
    };

    /**
     * @brief Structure holding window creation parameters.
     */
    struct WindowInfo
    {
        int width;         ///< Initial window width in pixels
        int height;        ///< Initial window height in pixels
        const char *title; ///< Window title
        WindowMode mode;   ///< Initial window mode
        bool vsync;        ///< Enable vertical synchronization
    };

    /**
     * @brief Represents an application window and provides abstraction over GLFW.
     *
     * Window handles creation, updating, mode switching, and querying properties
     * such as size, aspect ratio, focus, and content scale.
     */
    class Window
    {
    public:
        /**
         * @brief Constructs a new window with the specified creation parameters.
         *
         * @param createInfo WindowInfo structure defining initial window settings.
         */
        Window(const WindowInfo &createInfo);

        /**
         * @brief Destroys the window and releases associated resources.
         */
        ~Window();

        CP_NO_COPY_CLASS(Window);

        /**
         * @brief Returns the underlying GLFWwindow handle.
         *
         * @return Pointer to the GLFWwindow.
         */
        GLFWwindow *GetWindowHandle() noexcept;

        /**
         * @brief Gets the current window mode.
         *
         * @return Reference to the WindowMode enum representing the window mode.
         */
        const WindowMode &GetWindowMode() const;

        /**
         * @brief Sets the window mode (Windowed, Borderless, Fullscreen).
         *
         * @param mode The desired WindowMode.
         */
        void SetWindowMode(const WindowMode &mode);

        /**
         * @brief Updates internal window state.
         *
         * Should be called every frame to process events and check for size changes.
         */
        void Update();

        /**
         * @brief Checks if the window has been requested to close.
         *
         * @return True if the window should close, false otherwise.
         */
        bool ShouldClose() const;

        /**
         * @brief Gets the current width of the window in pixels.
         *
         * @return Window width.
         */
        int GetWidth() const;

        /**
         * @brief Gets the current height of the window in pixels.
         *
         * @return Window height.
         */
        int GetHeight() const;

        /**
         * @brief Calculates the window's aspect ratio (width / height).
         *
         * @return Aspect ratio as a floating-point value.
         */
        f32 GetAspectRatio() const;

        /**
         * @brief Checks if the window currently has focus.
         *
         * @return True if focused, false otherwise.
         */
        bool IsFocused() const;

        /**
         * @brief Sets the window title.
         *
         * @param title New window title string.
         */
        void SetTitle(const string &title);

        /**
         * @brief Sets the window opacity (alpha).
         *
         * @param alpha Value between 0.0 (transparent) and 1.0 (opaque).
         */
        void SetOpacity(f32 alpha);

        /**
         * @brief Sets whether the window should always stay on top of other windows.
         *
         * @param enable True to enable always-on-top, false to disable.
         */
        void SetAlwaysOnTop(bool enable);

        /**
         * @brief Sets the clipboard text.
         *
         * @param text Null-terminated string to store in the system clipboard.
         */
        void SetClipboardText(const char *text);

        /**
         * @brief Retrieves the current clipboard text.
         *
         * @return Clipboard contents as a string.
         */
        string GetClipboardText() const;

        /**
         * @brief Retrieves the content scale of the window for HiDPI displays.
         *
         * @param x Output scale factor in the X axis.
         * @param y Output scale factor in the Y axis.
         */
        void GetContentScale(f32 &x, f32 &y) const;

        /**
         * @brief Checks whether vertical synchronization (VSync) is enabled.
         *
         * @return True if VSync is enabled, false otherwise.
         */
        bool IsVSyncEnabled() const;

        /**
         * @brief Enables or disables vertical synchronization.
         *
         * @param enabled True to enable VSync, false to disable.
         */
        void SetVSyncEnabled(bool enabled);

    private:
        // --------------------------------------------------------
        // GLFW callbacks
        // --------------------------------------------------------

        /**
         * @brief Callback triggered when the window size changes.
         */
        static void GLFW_WindowSizeCallback(GLFWwindow *window, int width, int height);

        /**
         * @brief Callback triggered when the window position changes.
         */
        static void GLFW_WindowPosCallback(GLFWwindow *window, int xpos, int ypos);

        /**
         * @brief Callback triggered when the window focus changes.
         */
        static void GLFW_WindowFocusCallback(GLFWwindow *window, int focused);

        // --------------------------------------------------------
        // Internal helper functions
        // --------------------------------------------------------

        /**
         * @brief Returns the monitor that currently contains the majority of the window.
         *
         * @param window Pointer to the GLFWwindow.
         * @return Pointer to the GLFWmonitor.
         */
        GLFWmonitor *getMonitorForWindow(GLFWwindow *window) const;

        /**
         * @brief Centers the window on the monitor.
         *
         * @param window Pointer to the GLFWwindow.
         */
        void centerWindowOnScreen(GLFWwindow *window) const;

        /**
         * @brief Internal implementation for setting the window mode.
         *
         * @param mode Desired WindowMode.
         */
        void setWindowMode(const WindowMode &mode);

    private:
        GLFWwindow *m_wndHandle = nullptr; ///< Pointer to the GLFW window
        WindowInfo m_wndInfo;              ///< Current window information

        int m_prevX = 0, m_prevY = 0, m_prevW = 0, m_prevH = 0; ///< Previous window position and size

        std::atomic<bool> m_sizeChanged{false};                       ///< Flag set when window size changed
        std::chrono::steady_clock::time_point m_sizeChangedTimePoint; ///< Time of last size change

        std::atomic<bool> m_isFocused{true}; ///< Whether the window is currently focused
    };
} // namespace cp
