#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <mutex>
#include "cp_framework/core/export.hpp"
#include "debug.hpp"

namespace cp
{
    /**
     * @class HighResolutionTimer
     * @brief Simple high-resolution timer utility using microsecond precision.
     *
     * Provides Start(), End() and a method to retrieve the elapsed time
     * in seconds. Internally uses steady_clock to avoid time jumps.
     */
    class HighResolutionTimer
    {
    public:
        /**
         * @brief Constructs a timer with initial zero timestamps.
         */
        HighResolutionTimer() : m_start(0), m_end(0) {}

        /**
         * @brief Starts the timer by recording the current timestamp.
         */
        void Start() { m_start = Now(); }

        /**
         * @brief Ends the timer by recording the current timestamp.
         */
        void End() { m_end = Now(); }

        /**
         * @brief Returns the elapsed time in seconds between Start() and End().
         *
         * @return Elapsed time in seconds.
         */
        [[nodiscard]] double GetElapsedSeconds() const
        {
            return static_cast<double>(m_end - m_start) * 1e-6; // microseconds → seconds
        }

    private:
        /**
         * @brief Retrieves the current high-resolution timestamp in microseconds.
         */
        static uint64_t Now()
        {
            using namespace std::chrono;
            return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
        }

        uint64_t m_start;
        uint64_t m_end;
    };

    /**
     * @class TimerSampler
     * @brief Collects performance samples and provides statistics.
     *
     * Samples represent timing values in milliseconds. The sampler computes:
     * - average
     * - minimum
     * - maximum
     * - total count
     *
     * It can optionally provide the full sample history.
     */
    class TimerSampler
    {
    public:
        /**
         * @brief Adds a new sample duration (in milliseconds).
         *
         * Updates min, max, average, and sample count incrementally.
         *
         * @param milliseconds The elapsed time to record.
         */
        void AddSample(double milliseconds)
        {
            m_samples.push_back(milliseconds);
            m_sampleCount++;

            if (milliseconds < m_min)
                m_min = milliseconds;
            if (milliseconds > m_max)
                m_max = milliseconds;

            // Incremental average update
            m_average = ((m_average * (m_sampleCount - 1)) + milliseconds) / m_sampleCount;
        }

        /// @return Average sample value.
        double GetAverage() const { return m_average; }
        /// @return Minimum recorded sample.
        double GetMin() const { return m_min; }
        /// @return Maximum recorded sample.
        double GetMax() const { return m_max; }
        /// @return Total sample count.
        size_t GetSampleCount() const { return m_sampleCount; }

        /**
         * @brief Returns the full history of recorded samples.
         *
         * @return Vector of sample values.
         */
        const std::vector<double> &GetSamples() const { return m_samples; }

    private:
        std::vector<double> m_samples;
        double m_average = 0.0;
        double m_min = std::numeric_limits<double>::max();
        double m_max = 0.0;
        size_t m_sampleCount = 0;
    };

    /**
     * @struct FrameData
     * @brief Holds real-time frame timing and FPS metrics.
     *
     * Contains:
     * - total frame count
     * - delta time information
     * - FPS metrics (current, average, min, max)
     */
    struct FrameData
    {
        uint64_t totalFrames = 0;

        /**
         * @struct TimeInfo
         * @brief Contains per-frame time metrics.
         */
        struct TimeInfo
        {
            double deltaTime = 0.0; ///< Delta time (seconds)
        } timeInfo;

        /**
         * @struct FpsInfo
         * @brief Contains FPS values.
         */
        struct FpsInfo
        {
            uint32_t current = 0;                                ///< FPS of the last frame
            uint32_t average = 0;                                ///< Running average FPS
            uint32_t min = std::numeric_limits<uint32_t>::max(); ///< Minimum FPS recorded
            uint32_t max = 0;                                    ///< Maximum FPS recorded
        } fpsInfo;
    };

    /**
     * @class FrameCounter
     * @brief Measures frame times and computes FPS metrics.
     *
     * Supports an initial warmup period where frame measurements
     * are ignored to avoid initialization spikes.
     */
    class FrameCounter
    {
    public:
        /**
         * @brief Creates the counter with the number of warmup frames.
         *
         * @param warmupFrames Number of frames to ignore before measuring.
         */
        explicit FrameCounter(size_t warmupFrames = 10)
            : m_frameCount(0), m_warmupFrames(warmupFrames), m_started(false) {}

        /**
         * @brief Marks the beginning of a frame.
         */
        void StartFrame()
        {
            if (m_started)
                return;
            m_started = true;
            m_lastTime = Now();
        }

        /**
         * @brief Marks the end of a frame and updates FPS metrics.
         */
        void EndFrame()
        {
            if (!m_started)
                return;

            const uint64_t now = Now();
            const double delta = static_cast<double>(now - m_lastTime) * 1e-6; // microseconds → seconds
            m_frameCount++;

            if (m_frameCount > m_warmupFrames)
            {
                m_frameData.timeInfo.deltaTime = delta;
                m_frameData.totalFrames++;

                uint32_t fps = static_cast<uint32_t>(1.0 / delta);
                m_frameData.fpsInfo.current = fps;

                if (m_frameData.totalFrames == 1)
                {
                    m_frameData.fpsInfo.average = fps;
                    m_frameData.fpsInfo.min = fps;
                    m_frameData.fpsInfo.max = fps;
                }
                else
                {
                    m_frameData.fpsInfo.average =
                        static_cast<uint32_t>(
                            (m_frameData.fpsInfo.average * (m_frameData.totalFrames - 1) + fps) /
                            m_frameData.totalFrames);

                    m_frameData.fpsInfo.min = std::min(m_frameData.fpsInfo.min, fps);
                    m_frameData.fpsInfo.max = std::max(m_frameData.fpsInfo.max, fps);
                }
            }

            m_started = false;
        }

        /**
         * @brief Returns all FPS and timing data collected so far.
         *
         * @return Reference to FrameData.
         */
        const FrameData &GetFrameData() const { return m_frameData; }

    private:
        /**
         * @brief Returns the current timestamp in microseconds.
         */
        static uint64_t Now()
        {
            using namespace std::chrono;
            return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
        }

        FrameData m_frameData;
        uint64_t m_lastTime = 0;
        bool m_started;
        size_t m_frameCount;
        size_t m_warmupFrames;
    };

    /**
     * @class DiagnosticsManager
     * @brief Central manager for performance diagnostics.
     *
     * Provides:
     * - FPS tracking via FrameCounter
     * - Named timers with aggregated statistics (TimerSampler)
     * - Per-frame begin/end tracking
     */
    class DiagnosticsManager
    {
    public:
        /**
         * @brief Constructs the diagnostics manager.
         *
         * @param warmupFrames Warmup frames passed to FrameCounter.
         */
        explicit DiagnosticsManager(size_t warmupFrames = 10)
            : m_frameCounter(warmupFrames) {}

        /// @brief Marks the beginning of a frame.
        void BeginFrame() { m_frameCounter.StartFrame(); }

        /// @brief Marks the end of a frame.
        void EndFrame() { m_frameCounter.EndFrame(); }

        /**
         * @brief Starts a named high-resolution timer.
         *
         * @param name Unique name of the timer.
         */
        void StartTimer(const std::string &name)
        {
            m_timerStartTimes[name] = Now();
        }

        /**
         * @brief Stops a named timer and records the elapsed time.
         *
         * @param name Timer name.
         */
        void StopTimer(const std::string &name)
        {
            auto it = m_timerStartTimes.find(name);
            if (it == m_timerStartTimes.end())
                return; // Timer was never started

            double elapsedMs = static_cast<double>(Now() - it->second) * 1e-3; // µs → ms
            m_timerSamplers[name].AddSample(elapsedMs);
            m_timerStartTimes.erase(it);
        }

        /**
         * @brief Returns real-time frame and FPS metrics.
         *
         * @return Reference to FrameData.
         */
        const FrameData &GetFrameData() const { return m_frameCounter.GetFrameData(); }

        /**
         * @brief Safe access to a TimerSampler by name.
         *
         * @param name Sampler name.
         * @return Reference to sampler or a dummy empty sampler.
         */
        const TimerSampler &GetTimerSampler(const std::string &name) const
        {
            static TimerSampler dummy;
            auto it = m_timerSamplers.find(name);
            return (it != m_timerSamplers.end()) ? it->second : dummy;
        }

        /**
         * @brief Returns a formatted summary including FPS and timer statistics.
         *
         * @return Human-readable diagnostics string.
         */
        std::string Summary() const
        {
            std::string out;
            const auto &fd = m_frameCounter.GetFrameData();

            out += "FPS " +
                   std::to_string(fd.fpsInfo.current) +
                   " (avg " + std::to_string(fd.fpsInfo.average) +
                   ", min " + std::to_string(fd.fpsInfo.min) +
                   ", max " + std::to_string(fd.fpsInfo.max) + ")\n";

            out += "Samplers:\n";
            for (const auto &[name, sampler] : m_timerSamplers)
            {
                out += "   *" + name + " : " +
                       std::to_string(sampler.GetAverage()) + " ms" +
                       " (min " + std::to_string(sampler.GetMin()) +
                       ", max " + std::to_string(sampler.GetMax()) + ")\n";
            }
            return out;
        }

    private:
        /**
         * @brief Returns current timestamp in microseconds.
         */
        static uint64_t Now()
        {
            using namespace std::chrono;
            return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
        }

        FrameCounter m_frameCounter;
        std::unordered_map<std::string, uint64_t> m_timerStartTimes;
        std::unordered_map<std::string, TimerSampler> m_timerSamplers;
    };
}
