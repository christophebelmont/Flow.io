/**
 * @file LogSerialSinkModule.cpp
 * @brief Implementation file.
 */
#include "LogSerialSinkModule.h"
#include <Arduino.h>

static const char* lvlStr(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::Debug: return "D";
        case LogLevel::Info:  return "I";
        case LogLevel::Warn:  return "W";
        case LogLevel::Error: return "E";
    }
    return "?";
}

static const char* lvlColor(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::Debug: return "\x1b[90m";
        case LogLevel::Info:  return "\x1b[32m";
        case LogLevel::Warn:  return "\x1b[33m";
        case LogLevel::Error: return "\x1b[31m";
    }
    return "";
}

static const char* colorReset() { return "\x1b[0m"; }

static bool isTimeValid()
{
    /// Méthode classique ESP32 : tant que NTP pas synchro, l'année reste ~1970
    time_t now = time(nullptr);
    return (now > 1609459200); ///< 2021-01-01 00:00:00
}

static void formatUptime(char *out, size_t outSize, uint32_t ms)
{
    uint32_t s   = ms / 1000;
    uint32_t m   = s / 60;
    uint32_t h   = m / 60;

    uint32_t hh  = h % 24;
    uint32_t mm  = m % 60;
    uint32_t ss  = s % 60;
    uint32_t mmm = ms % 1000;

    snprintf(out, outSize, "%02lu:%02lu:%02lu.%03lu",
             (unsigned long)hh,
             (unsigned long)mm,
             (unsigned long)ss,
             (unsigned long)mmm);
}

static void serialSinkWrite(void* ctx, const LogEntry& e) {
    (void)ctx;

    char ts[48];

    if (isTimeValid()) {
        /// Heure réelle (NTP OK)
        time_t now = time(nullptr);
        struct tm t;
        localtime_r(&now, &t);

        /// Si tu veux inclure les millisecondes dans l'heure réelle:
        /// on utilise e.ts_ms % 1000 comme fraction (supposé venir de millis/horloge interne)
        unsigned ms = e.ts_ms % 1000;

        snprintf(ts, sizeof(ts),
                 "%04d-%02d-%02d %02d:%02d:%02d.%03u",
                 t.tm_year + 1900,
                 t.tm_mon + 1,
                 t.tm_mday,
                 t.tm_hour,
                 t.tm_min,
                 t.tm_sec,
                 ms);
    } else {
        /// Fallback uptime
        formatUptime(ts, sizeof(ts), e.ts_ms);
    }

    const char* color = lvlColor(e.lvl);
    Serial.printf("[%s][%s][%s] %s%s%s\n",
                  ts,
                  lvlStr(e.lvl),
                  e.tag,
                  color,
                  e.msg,
                  colorReset());
}

void LogSerialSinkModule::init(ConfigStore& cfg, ServiceRegistry& services) {
    (void)cfg;

    Serial.begin(115200);

    auto sinks = services.get<LogSinkRegistryService>("logsinks");
    if (!sinks) return;

    LogSinkService sink{};
    sink.write = serialSinkWrite;
    sink.ctx = nullptr;

    sinks->add(sinks->ctx, sink);
}
