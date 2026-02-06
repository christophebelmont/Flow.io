#pragma once
/**
 * @file LogSinkRegistry.h
 * @brief Registry of log sinks.
 */
#include "Core/Services/ILogger.h"

/**
 * @brief Stores and enumerates registered log sinks.
 */
class LogSinkRegistry {
public:
    /** @brief Add a sink to the registry. */
    bool add(LogSinkService sink);
    /** @brief Number of registered sinks. */
    int count() const;
    /** @brief Get sink by index. */
    LogSinkService get(int idx) const;

private:
    static constexpr int MAX_SINKS = 4;
    LogSinkService sinks[MAX_SINKS]{};
    int n = 0;
};
