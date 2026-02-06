/**
 * @file LogSinkRegistry.cpp
 * @brief Implementation file.
 */
#include "Core/LogSinkRegistry.h"
#define LOG_TAG_CORE "LogSinkR"

bool LogSinkRegistry::add(LogSinkService sink) {
    if (n >= MAX_SINKS) return false;
    sinks[n++] = sink;
    return true;
}

int LogSinkRegistry::count() const {
    return n;
}

LogSinkService LogSinkRegistry::get(int idx) const {
    if (idx < 0 || idx >= n) return LogSinkService{};
    return sinks[idx];
}
