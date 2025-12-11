#pragma once

#include <cstdint>
#include <chrono>
#include "cp_framework/core/export.hpp"
#include "cp_framework/core/types.hpp"

namespace cp
{
    /**
     * @brief Represents timing information for a single frame update and fixed-step updates.
     *
     * GameTime is responsible for tracking delta time (time between frames),
     * total elapsed time, frame counters, and fixed-step updates for physics or logic.
     * Updated once per frame via Update().
     */
    class GameTime
    {
        /**
         * @brief Constructs a new GameTime instance.
         *
         * Initializes timestamps, counters, and the fixed timestep.
         */
        GameTime(f64 fixedDeltaSeconds = 1.0 / 60.0)
            : m_deltaTime(0.0),
              m_unscaledDeltaTime(0.0),
              m_totalTime(0.0),
              m_unscaledTotalTime(0.0),
              m_fixedDeltaTime(fixedDeltaSeconds),
              m_accumulator(0.0),
              m_timeScale(1.0),
              m_maxDeltaClamp(0.25),
              m_frameCount(0),
              m_paused(false)
        {
            m_lastTime = Clock::now();
        }

    public:
        MAKE_SINGLETON(GameTime);
        CP_NO_COPY_CLASS(GameTime);

        /**
         * @brief Updates timing values. Must be called once per frame.
         *
         * Calculates delta time since the previous frame, increments frame counters,
         * and accumulates total running time unless paused. Also accumulates
         * delta for fixed-step updates.
         */
        void Update()
        {
            auto now = Clock::now();
            std::chrono::duration<f64> dt = now - m_lastTime;
            m_lastTime = now;

            // Clamp delta to avoid huge spikes
            m_unscaledDeltaTime = std::min(dt.count(), m_maxDeltaClamp);

            if (!m_paused)
            {
                m_deltaTime = m_unscaledDeltaTime * m_timeScale;
                m_totalTime += m_deltaTime;
                m_unscaledTotalTime += m_unscaledDeltaTime;
                m_accumulator += m_deltaTime;
            }
            else
            {
                m_deltaTime = 0.0;
            }

            m_frameCount++;
        }

        /**
         * @brief Determines whether a fixed update should be processed.
         *
         * Returns true if enough accumulated time has passed for one fixed timestep.
         * Each call consumes one fixed delta. Call repeatedly to catch up if multiple
         * fixed steps are required in a single frame.
         *
         * @return True if a fixed update should occur, otherwise false.
         */
        bool DoFixedUpdate()
        {
            if (m_paused)
                return false;

            if (m_accumulator >= m_fixedDeltaTime)
            {
                m_accumulator -= m_fixedDeltaTime;
                return true;
            }
            return false;
        }

        /**
         * @brief Gets the delta time in seconds.
         *
         * Delta time represents the time elapsed since the last frame.
         *
         * @return Time between frames, in seconds.
         */
        CP_API f64 DeltaTime() const { return m_deltaTime; }

        /**
         * @brief Gets the unscaled delta time in seconds.
         *
         * @return Unscaled delta time
         */
        CP_API f64 UnscaledDeltaTime() const { return m_unscaledDeltaTime; }

        /**
         * @brief Gets the total elapsed time since the game/application started.
         *
         * @return Total running time in seconds.
         */
        CP_API f64 TotalTime() const { return m_totalTime; }

        /**
         * @brief Gets the total unscaled time.
         *
         * @return the total unscaled time.
         */
        CP_API f64 UnscaledTotalTime() const { return m_unscaledTotalTime; }

        /**
         * @brief Gets the fixed delta time in seconds.
         *
         * @return The fixed timestep for fixed updates.
         */
        CP_API f64 FixedDeltaTime() const { return m_fixedDeltaTime; }

        /**
         * @brief Sets the fixed delta time in seconds.
         *
         * @param seconds Fixed timestep for physics/logic updates.
         */
        CP_API void SetFixedDeltaTime(f64 seconds) { m_fixedDeltaTime = seconds; }

        /**
         * @brief Gets the accumulated frame count.
         *
         * @return Number of frames that have been processed.
         */
        CP_API uint64_t FrameCount() const { return m_frameCount; }

        /**
         * @brief Get the Frame por seconds
         *
         * @returns the FPS
         */
        CP_API f64 FPS() const { return m_unscaledDeltaTime > 0.0 ? 1.0 / m_unscaledDeltaTime : 0.0; }

        /**
         * @brief Set the time scale
         *
         */
        CP_API void SetTimeScale(f64 scale) { m_timeScale = std::max(0.0, scale); }

        /**
         * @brief Get the Time Scale;
         *
         * @return the time scale.
         */
        CP_API f64 TimeScale() const { return m_timeScale; }

        /**
         * @brief Pauses all time progression.
         *
         * When paused, deltaTime becomes zero and totalTime does not advance.
         */
        CP_API void Pause() { m_paused = true; }

        /**
         * @brief Resumes normal time progression after being paused.
         */
        CP_API void Resume() { m_paused = false; }

        /**
         * @brief Checks whether the time system is currently paused.
         *
         * @return True if paused; otherwise false.
         */
        CP_API bool IsPaused() const { return m_paused; }

        /**
         * @brief Resets timing information.
         *
         * Delta time becomes zero, total time resets to zero, fixed accumulator is cleared,
         * and frame counters restart from zero. Next Update() call restarts the timer.
         */
        void Reset()
        {
            m_deltaTime = 0.0;
            m_unscaledDeltaTime = 0.0;
            m_totalTime = 0.0;
            m_unscaledTotalTime = 0.0;
            m_accumulator = 0.0;
            m_frameCount = 0;
            m_lastTime = Clock::now();
        }

    private:
        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point m_lastTime; ///< Timestamp of last frame

        f64 m_deltaTime;         ///< Delta time scaled
        f64 m_unscaledDeltaTime; ///< Delta time real
        f64 m_totalTime;         ///< Total scaled time
        f64 m_unscaledTotalTime; ///< Total unscaled time
        f64 m_fixedDeltaTime;    ///< Fixed timestep
        f64 m_accumulator;       ///< Accumulated time for fixed updates
        f64 m_timeScale;         ///< Time scale multiplier
        f64 m_maxDeltaClamp;     ///< Max delta clamp to prevent spikes

        uint64_t m_frameCount; ///< Frame counter
        bool m_paused;         ///< Is time paused
    };
} // namespace cp
