#pragma once

#include <string>
#include <fmt/core.h>
#include <fmt/std.h>
#include <ostream>
#include <mutex>

namespace cp {

    /**
     * @enum LogLevel
     * @brief Represents the severity level used for logging output.
     *
     * Levels are used to filter messages and apply different color formatting.
     * Higher-severity messages (Error, Warn) are typically always shown,
     * while lower levels (Debug) can be enabled or hidden depending on build mode.
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
     * @brief Logging utility class providing color output, filtering, and thread safety.
     *
     * Features:
     * - Configurable minimum log level.
     * - Automatic flush control.
     * - Optional ANSI color support (auto-handled on Windows).
     * - Thread-safe output using an internal mutex.
     * - Optional redirection to a file instead of the console.
     *
     * This class is fully static and does not require instantiation.
     */
    class Debug {
    public:
        /**
         * @brief Enables or disables ANSI color output.
         *
         * @param enabled True to enable colored output, false to disable.
         */
        static void SetColorEnabled(bool enabled);

        /**
         * @brief Sets the minimum log level that will be printed.
         *
         * Messages below this level will be ignored.
         *
         * @param level Minimum log level.
         */
        static void SetMinimumLevel(LogLevel level);

        /**
         * @brief Enables or disables automatic flushing after each log message.
         *
         * @param enabled True to flush automatically, false to buffer output.
         */
        static void SetAutoFlush(bool enabled);

        /**
         * @brief Redirects all output to a specific file.
         *
         * Any previously opened file stream is replaced.
         *
         * @param filepath Path to the file to be used for logging.
         */
        static void SetLogFile(const std::string& filepath);

        /**
         * @brief Restores output back to the standard console (stdout).
         */
        static void ResetOutputToConsole();

        /**
         * @brief Generic logging function with fmt-style formatting.
         *
         * @tparam Args Format arguments.
         * @param level Log severity to classify the message.
         * @param format Format string compatible with fmt::format.
         * @param args Arguments substituted into the format string.
         */
        template <typename... Args>
        static void Log(LogLevel level, fmt::format_string<Args...> format, Args&&... args) {
            if (level < g_minLevel) return;

            const std::string msg = fmt::format(format, std::forward<Args>(args)...);
            Print(level, msg);
        }

        /**
         * @brief Logs an error and throws a std::runtime_error with the same message.
         *
         * Useful for exceptions while preserving consistent logging behavior.
         *
         * @tparam Args Format arguments.
         * @param format Format string.
         * @param args Arguments for the formatted message.
         *
         * @throws std::runtime_error Always thrown with the formatted message.
         */
        template<typename... Args>
        static void Throw(fmt::format_string<Args...> format, Args&&... args) {
            const std::string msg = fmt::format(format, std::forward<Args>(args)...);
            Print(LogLevel::Error, msg);
            throw std::runtime_error(msg);
        }

        /**
         * @brief Internal function responsible for writing the final formatted message.
         *
         * Applies coloring, locking, prefixing, and optional flushing based on
         * current Debug settings.
         *
         * @param level Severity of the message.
         * @param message Final message to output.
         */
        static void Print(LogLevel level, const std::string& message);

    private:
        /// Whether ANSI colors are currently enabled.
        static inline bool g_colorEnabled = true;

        /// Whether the logger flushes automatically after each message.
        static inline bool g_autoFlush = true;

        /// Minimum log level to display.
        static inline LogLevel g_minLevel =
    #ifndef NDEBUG
            LogLevel::Info;  ///< In Debug builds, show all messages.
    #else
            LogLevel::Warn;  ///< In Release builds, hide Info/Debug by default.
    #endif

        /// Output stream used for logging (defaults to stdout).
        static inline std::ostream* g_outputStream = nullptr;

        /// Mutex providing thread-safe logging.
        static inline std::mutex g_mutex;
    };

} // namespace cp

// -----------------------------------------------------------------------------
// Simplified logging macros
// -----------------------------------------------------------------------------

/**
 * @brief Logs an informational message.
 */
#define LOG_INFO(fmt_str, ...)    ::cp::Debug::Log(::cp::LogLevel::Info, fmt_str, ##__VA_ARGS__)

/**
 * @brief Logs a success message.
 */
#define LOG_SUCCESS(fmt_str, ...) ::cp::Debug::Log(::cp::LogLevel::Success, fmt_str, ##__VA_ARGS__)

/**
 * @brief Logs a warning message.
 */
#define LOG_WARN(fmt_str, ...)    ::cp::Debug::Log(::cp::LogLevel::Warn, fmt_str, ##__VA_ARGS__)

/**
 * @brief Logs an error message.
 */
#define LOG_ERROR(fmt_str, ...)   ::cp::Debug::Log(::cp::LogLevel::Error, fmt_str, ##__VA_ARGS__)

/**
 * @brief Logs a debug-only message.
 */
#define LOG_DEBUG(fmt_str, ...)   ::cp::Debug::Log(::cp::LogLevel::Debug, fmt_str, ##__VA_ARGS__)

/**
 * @brief Logs an error and throws a runtime exception.
 */
#define LOG_THROW(fmt_str, ...)   ::cp::Debug::Throw(fmt_str, ##__VA_ARGS__)
