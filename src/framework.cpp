#include "cp_framework/framework.hpp"
#include "cp_framework/core/debug.hpp"
#include "cp_framework/systems/eventSystem.hpp"
#include "cp_framework/core/gameTime.hpp"
#include "cp_framework/graphics/window.hpp"

namespace cp
{
    Framework::Framework()
    {
        ScopedLog("FRAMEWORK", "Creating framework class", "Successfully created framework class");
        // singletons initialization.
        EventSystem::Get();
        GameTime::Get();
    }

    Framework::~Framework()
    {
        ScopedLog("FRAMEWORK", "Destroying framework class", "Successfully destroyed framework class");
    }

    void Framework::Init()
    {
        ScopedLog("FRAMEWORK", "Starting to initialize.", "Successfully initialized.");

        // Create modules
        WindowInfo createInfo{.width = 1320, .height = 780, .title = "CP_FRAMEWORK", .mode = WindowMode::Windowed, .vsync = true};
        m_window = M_UPTR<Window>(createInfo);

        // set init = true
        m_initializated = true;
    }

    void Framework::Run()
    {
        assert(m_initializated && "Init function must be called before Run func");

        ScopedLog("FRAMEWORK", "Running game loop.", "Successfully stopped game loop.");
        m_isRunning.store(true);

        while (m_isRunning.load())
        {
            // -----------------------------
            // Update window
            // -----------------------------
            m_window->Update();
            if (m_window->ShouldClose())
            {
                m_isRunning.store(false);
                break;
            }

            // -----------------------------
            // Update global game time
            // -----------------------------
            GameTime &gameTime = GameTime::Get();
            gameTime.Update();
            f64 dt = gameTime.DeltaTime();

            // -----------------------------
            // Per-frame update
            // -----------------------------
            // Update(dt);

            // -----------------------------
            // Fixed update (physics, logic)
            // -----------------------------
            while (gameTime.DoFixedUpdate())
            {
                // FixedUpdate(gameTime.FixedDeltaTime())
            }

            // -----------------------------
            // Late update / rendering
            // -----------------------------
            // do late update...
        }
    }
} // namespace cp
