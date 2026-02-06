#pragma once
/**
 * @file LogDispatcherModule.h
 * @brief Module that dispatches log entries to sinks.
 */
#include "Core/ModulePassive.h"
#include "Core/ServiceRegistry.h"
#include "Core/Services/ILogger.h"
#include "Core/LogHub.h"

/**
 * @brief Passive module that runs a task to consume log hub entries.
 */
class LogDispatcherModule : public ModulePassive {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "log.dispatcher"; }

    /** @brief Depends on log hub. */
    uint8_t dependencyCount() const override { return 1; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        return nullptr;
    }

    /** @brief Start dispatcher task and wire sink registry. */
    void init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) override;

private:
    static void taskFn(void* pv);

    ServiceRegistry* _services = nullptr;
    LogHub* _hub = nullptr;
    const LogSinkRegistryService* _sinkReg = nullptr;
};
