#pragma once
/**
 * @file ILogger.h
 * @brief Logging service interfaces and helpers.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>

/** @brief Log severity levels. */
enum class LogLevel : uint8_t { Debug, Info, Warn, Error };

// ===== LOG TYPES =====
constexpr int LOG_TAG_MAX = 10;
constexpr int LOG_MSG_MAX = 110;

/** @brief Fixed-size log entry. */
struct LogEntry {
    uint32_t ts_ms;
    LogLevel lvl;
    char tag[LOG_TAG_MAX];
    char msg[LOG_MSG_MAX];
};

/** @brief Log sink interface. */
struct LogSinkService {
    void (*write)(void* ctx, const LogEntry& e);
    void* ctx;
};

/** @brief Log hub interface (producer side). */
struct LogHubService {
    bool (*enqueue)(void* ctx, const LogEntry& e);
    void* ctx;
};

/** @brief Registry interface for log sinks. */
struct LogSinkRegistryService {
    bool (*add)(void* ctx, LogSinkService sink);
    int (*count)(void* ctx);
    LogSinkService (*get)(void* ctx, int index);
    void* ctx;
};

/** @brief Helper to format and enqueue a log entry. */
static inline void LOGHUBF(const LogHubService* s,
                           LogLevel lvl,
                           const char* tag,
                           const char* fmt, ...) {
    if (!s || !s->enqueue) return;

    LogEntry e{};
    e.ts_ms = millis();
    e.lvl = lvl;

    if (tag) {
        strncpy(e.tag, tag, LOG_TAG_MAX - 1);
    } else {
        strncpy(e.tag, "-", LOG_TAG_MAX - 1);
    }

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(e.msg, LOG_MSG_MAX, fmt, ap);
    va_end(ap);

    // non bloquant (la queue peut drop si pleine)
    s->enqueue(s->ctx, e);
}
