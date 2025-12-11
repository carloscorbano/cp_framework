#pragma once

#include "core/export.hpp"
#include "core/types.hpp"
#include <atomic>

namespace cp
{
    //-------------------------
    // FORWARD DECLARATIONS
    //-------------------------
    class Window;
    class ThreadPool;
    class DiagnosticsManager;
    class InputManager;
    class VkManager;

    /**
     * @brief Framework class, this class controll the framework creation and game loop
     */
    class CP_API Framework
    {
    public:
        Framework();
        ~Framework();

        CP_NO_COPY_CLASS(Framework);

        /**
         * @brief Initializes the framework.
         */
        void Init();

        /**
         * @brief Starts the main game loop.
         */
        void Run();

    private:
        void update(const f64 &deltaTime);
        void fixedUpdate(const f64 &fixedTime);
        void lateUpdate(const f64 &deltaTime);

    private:
        bool m_initializated{false};
        std::atomic<bool> m_isRunning{false};

        UPTR<Window> m_window;
        UPTR<ThreadPool> m_threadPool;
        UPTR<DiagnosticsManager> m_diag;
        UPTR<InputManager> m_input;
        UPTR<VkManager> m_vkManager;
    };
} // namespace cp
