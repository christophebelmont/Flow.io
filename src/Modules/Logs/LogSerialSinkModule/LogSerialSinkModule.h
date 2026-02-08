#pragma once
/**
 * @file LogSerialSinkModule.h
 * @brief Serial log sink module.
 */
#include "Core/ModulePassive.h"
#include "Core/Services/ILogger.h"
#include "Core/Services/INTP.h"     // To know if NTP is synced
#include "Core/ServiceRegistry.h"

/**
 * @brief Passive module that writes log entries to Serial.
 */
class LogSerialSinkModule : public ModulePassive {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "log.sink.serial"; }

    /** @brief Depends on log hub. */
    uint8_t dependencyCount() const override { return 1; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        return nullptr;
    }

    /** @brief Register the serial log sink. */
    void init(ConfigStore& cfg, ServiceRegistry& services) override;
};
