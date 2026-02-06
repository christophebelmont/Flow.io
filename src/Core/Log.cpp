/**
 * @file Log.cpp
 * @brief Implementation file.
 */
#include "Core/Log.h"
#include <cstring>
#include <stdarg.h>
#include <stdio.h>

namespace {
    const LogHubService* g_hub = nullptr;

    void logVa(LogLevel lvl, const char* tag, const char* fmt, va_list ap) {
        if (!g_hub || !g_hub->enqueue || !fmt) return;

        LogEntry e{};
        e.ts_ms = millis();
        e.lvl = lvl;

        if (tag) {
            strncpy(e.tag, tag, LOG_TAG_MAX - 1);
        } else {
            strncpy(e.tag, "-", LOG_TAG_MAX - 1);
        }

        vsnprintf(e.msg, LOG_MSG_MAX, fmt, ap);
        g_hub->enqueue(g_hub->ctx, e);
    }
}

void Log::setHub(const LogHubService* hub) {
    g_hub = hub;
}

const LogHubService* Log::hub() {
    return g_hub;
}

void Log::logf(LogLevel lvl, const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logVa(lvl, tag, fmt, ap);
    va_end(ap);
}

void Log::debug(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logVa(LogLevel::Debug, tag, fmt, ap);
    va_end(ap);
}

void Log::info(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logVa(LogLevel::Info, tag, fmt, ap);
    va_end(ap);
}

void Log::warn(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logVa(LogLevel::Warn, tag, fmt, ap);
    va_end(ap);
}

void Log::error(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logVa(LogLevel::Error, tag, fmt, ap);
    va_end(ap);
}
