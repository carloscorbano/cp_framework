#pragma once

#include <string>
#include <fmt/core.h>
#include <fmt/std.h>
#include <ostream>
#include <mutex>

/**
 * @defgroup Logging Logging System
 * @brief Thread-safe logging utilities with color output and formatting.
 *
 * This module provides:
 * - Log levels
 * - A thread-safe static logger class
 * - ANSI color handling
 * - Optional file output redirection
 * - Convenience logging macros
 * @{
 */

namespace cp {

    /**
     * @enum LogLevel
     * @brief Represents the severity level used for logging output.
     *
     * Levels are used to filter messages and apply different color formatting.
     */
    enum class LogLevel {
        Info,    ///< Informational message.
        Success, ///< Indicates a successful operation.
        Warn,    ///< Warning: non-critical issue.
        Error,   ///< Error: critical failure or unexpected condition.
        Debug    ///< Debug-only messages.
    };

    /**
     * @class Debug
     * @brief Static logging utility class providing formatting, thread safety, and output control.
     *
     * Features:
     * - Configurable minimum log level.
     * - Automatic flush control.
     * - Optional ANSI color support.
     * - Thread-safe output using an internal mutex.
     * - Optional redirection to a file instead of the console.
     *
     * @ingroup Logging
     */
    class Debug {
    public:
        /** @brief Enables or disables ANSI color output. */
        static void SetColorEnabled(bool enabled);

        /** @brief Sets the minimum log level that will be printed. */
        static void SetMinimumLevel(LogLevel level);

        /** @brief Enables or disables automatic flushing after each log message. */
        static void SetAutoFlush(bool enabled);

        /** @brief Redirects all output to a file. */
        static void SetLogFile(const std::string& filepath);

        /** @brief Restores output back to stdout. */
        static void ResetOutputToConsole();

        /**
         * @brief Generic logging function with fmt-style formatting.
         *
         * @tparam Args Format arguments.
         * @param level Log severity.
         * @param format Format string.
         * @param args Arguments substituted into the format.
         *
         * @ingroup Logging
         */
        template <typename... Args>
        static void Log(LogLevel level, fmt::format_string<Args...> format, Args&&... args) {
            if (level < g_minLevel) return;

            const std::string msg = fmt::format(format, std::forward<Args>(args)...);
            Print(level, msg);
        }

        /**
         * @brief Logs an error and throws a std::runtime_error.
         *
         * @throws std::runtime_error Always thrown.
         *
         * @ingroup Logging
         */
        template<typename... Args>
        static void Throw(fmt::format_string<Args...> format, Args&&... args) {
            const std::string msg = fmt::format(format, std::forward<Args>(args)...);
            Print(LogLevel::Error, msg);
            throw std::runtime_error(msg);
        }

        /**
         * @brief Internal function responsible for writing the formatted message.
         *
         * Applies coloring, locking, and optional flushing.
         *
         * @ingroup Logging
         */
        static void Print(LogLevel level, const std::string& message);

    private:
        static inline bool g_colorEnabled = true;      ///< ANSI color toggle
        static inline bool g_autoFlush = true;         ///< Auto flush toggle

#ifndef NDEBUG
        static inline LogLevel g_minLevel = LogLevel::Info;  ///< Debug build default
#else
        static inline LogLevel g_minLevel = LogLevel::Warn;  ///< Release build default
#endif

        static inline std::ostream* g_outputStream = nullptr; ///< Output stream
        static inline std::mutex g_mutex;                      ///< Thread safety mutex
    };

} // namespace cp

/** @} */ // end of Logging group


// -----------------------------------------------------------------------------
// Macros Group
// -----------------------------------------------------------------------------

/**
 * @defgroup Logging_Macros Logging Macros
 * @brief Convenience macros that wrap cp::Debug functions.
 * @ingroup Logging
 * @{
 */

/** @brief Logs an informational message. */
#define LOG_INFO(fmt_str, ...)    ::cp::Debug::Log(::cp::LogLevel::Info,    fmt_str, ##__VA_ARGS__)

/** @brief Logs a success message. */
#define LOG_SUCCESS(fmt_str, ...) ::cp::Debug::Log(::cp::LogLevel::Success, fmt_str, ##__VA_ARGS__)

/** @brief Logs a warning. */
#define LOG_WARN(fmt_str, ...)    ::cp::Debug::Log(::cp::LogLevel::Warn,    fmt_str, ##__VA_ARGS__)

/** @brief Logs an error. */
#define LOG_ERROR(fmt_str, ...)   ::cp::Debug::Log(::cp::LogLevel::Error,   fmt_str, ##__VA_ARGS__)

/** @brief Logs a debug-only message. */
#define LOG_DEBUG(fmt_str, ...)   ::cp::Debug::Log(::cp::LogLevel::Debug,   fmt_str, ##__VA_ARGS__)

/** @brief Logs an error and throws a runtime exception. */
#define LOG_THROW(fmt_str, ...)   ::cp::Debug::Throw(fmt_str, ##__VA_ARGS__)

/** @} */ // end of Logging_Macros group
