#include "cp_framework/framework.hpp"

#include "cp_framework/debug/debug.hpp"
#include "cp_framework/debug/diagnostics.hpp"
#include "cp_framework/events/eventSystem.hpp"
#include "cp_framework/time/gameTime.hpp"
#include "cp_framework/window/window.hpp"
#include "cp_framework/threading/threadPool.hpp"
#include "cp_framework/input/inputManager.hpp"
#include "cp_framework/vulkan/manager.hpp"

namespace cp
{
    Framework::Framework()
    {
        ScopedLog slog("FRAMEWORK", "Creating framework class", "Successfully created framework class");
        // singletons initialization.
        EventSystem::Get();
        GameTime::Get();
    }

    Framework::~Framework()
    {
        ScopedLog slog("FRAMEWORK", "Destroying framework class", "Successfully destroyed framework class");
    }

    void Framework::Init()
    {
        ScopedLog slog("FRAMEWORK", "Starting to initialize.", "Successfully initialized.");

        // Create modules
        WindowInfo createInfo{.width = 1320, .height = 780, .title = "CP_FRAMEWORK", .mode = WindowMode::Windowed, .vsync = true};
        m_window = M_UPTR<Window>(createInfo);
        m_threadPool = M_UPTR<ThreadPool>();
        m_diag = M_UPTR<DiagnosticsManager>();
        m_input = M_UPTR<InputManager>(m_window->GetWindowHandle());
        m_vkManager = M_UPTR<VkManager>(m_window->GetWindowHandle());

        // set init = true
        m_initializated = true;
    }

    void Framework::Run()
    {
        assert(m_initializated && "Init function must be called before Run func");
        LOG_INFO("[FRAMEWORK] Running main game loop!");
        m_isRunning.store(true);

        while (m_isRunning.load())
        {
            m_diag->BeginFrame();

            // -----------------------------
            // modules update
            // -----------------------------
            m_window->Update();
            if (m_window->ShouldClose())
            {
                m_isRunning.store(false);
                break;
            }

            m_input->update();

            // -----------------------------
            // Update global game time
            // -----------------------------
            GameTime &gameTime = GameTime::Get();
            gameTime.Update();
            f64 dt = gameTime.DeltaTime();

            // -----------------------------
            // Per-frame update
            // -----------------------------
            update(dt);

            // -----------------------------
            // Fixed update (physics, logic)
            // -----------------------------
            while (gameTime.DoFixedUpdate())
            {
                fixedUpdate(gameTime.FixedDeltaTime());
            }

            // -----------------------------
            // Late update / rendering
            // -----------------------------
            lateUpdate(dt);

            m_diag->EndFrame();
        }

        LOG_SUCCESS("[FRAMEWORK] Successfully terminanted game loop!");
    }

    void Framework::update(const f64 &deltaTime)
    {
        (void)deltaTime;
        if (m_input->isKeyPressed(GLFW_KEY_A))
        {
            m_vkManager->GetSwapchain().Recreate(VK_PRESENT_MODE_FIFO_KHR);
        }
    }

    void Framework::fixedUpdate(const f64 &fixedTime)
    {
        (void)fixedTime;
    }

    void Framework::lateUpdate(const f64 &deltaTime)
    {
        (void)deltaTime;
    }

} // namespace cp
