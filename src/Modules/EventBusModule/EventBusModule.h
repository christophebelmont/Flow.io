#pragma once
/**
 * @file EventBusModule.h
 * @brief Active module hosting the EventBus task/dispatch loop.
 */
#include "Core/Module.h"
#include "Core/Services/Services.h"
#include "Core/EventBus/EventBus.h"

/**
 * @brief Active module that owns the EventBus instance.
 */
class EventBusModule : public Module {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "eventbus"; }
    /** @brief Task name. */
    const char* taskName() const override { return "EventBus"; }

    /** @brief EventBus depends on log hub. */
    uint8_t dependencyCount() const override { return 1; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        return nullptr;
    }
    /** @brief Initialize and register EventBus service. */
    void init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) override;
    /** @brief Dispatch events from the queue. */
    void loop() override;

    /** @brief Stack size override. */
    uint16_t taskStackSize() const override { return 4096; }
    /** @brief Task priority override. */
    UBaseType_t taskPriority() const override { return 1; }

private:
    EventBus _bus;
    EventBusService _svc { &_bus };

    const LogHubService* logHub = nullptr;
};
