#pragma once
/**
 * @file LogAlarmSinkModule.h
 * @brief Log sink that maps warning/error logs to alarm conditions.
 */

#include "Core/ModulePassive.h"
#include "Core/Services/Services.h"
#include <freertos/FreeRTOS.h>

class LogAlarmSinkModule : public ModulePassive {
public:
    const char* moduleId() const override { return "log.sink.alarm"; }

    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "alarms";
        return nullptr;
    }

    void init(ConfigStore& cfg, ServiceRegistry& services) override;

private:
    static constexpr uint32_t WARN_HOLD_MS = 60000U;
    static constexpr uint32_t ERROR_HOLD_MS = 120000U;

    struct SinkState {
        uint32_t lastWarnMs = 0;
        uint32_t lastErrorMs = 0;
        portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    };

    static void sinkWrite_(void* ctx, const LogEntry& e);
    static AlarmCondState condWarn_(void* ctx, uint32_t nowMs);
    static AlarmCondState condError_(void* ctx, uint32_t nowMs);
    static bool ignoredTag_(const char* tag);

    SinkState state_{};
};
