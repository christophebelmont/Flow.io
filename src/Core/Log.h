/**
 * @file Log.h
 * @brief Global log helper for core and modules.
 */
#pragma once

#include "Core/Services/ILogger.h"

namespace Log {
    /**
     * @brief Set the global log hub service.
     */
    void setHub(const LogHubService* hub);

    /**
     * @brief Get the current global log hub service.
     */
    const LogHubService* hub();

    /**
     * @brief Log a formatted message with a given level.
     */
    void logf(LogLevel lvl, const char* tag, const char* fmt, ...);

    /** @brief Convenience: Debug log. */
    void debug(const char* tag, const char* fmt, ...);
    /** @brief Convenience: Info log. */
    void info(const char* tag, const char* fmt, ...);
    /** @brief Convenience: Warning log. */
    void warn(const char* tag, const char* fmt, ...);
    /** @brief Convenience: Error log. */
    void error(const char* tag, const char* fmt, ...);
}

// Macros are provided by Core/ModuleLog.h to keep Module.h neutral.
