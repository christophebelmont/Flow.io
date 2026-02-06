/**
 * @file ActuatorsModule.cpp
 * @brief Implementation file.
 */
#include "ActuatorsModule.h"
#include "Modules/Actuators/ActuatorsRuntime.h"
#define LOG_TAG "Actuators"
#include "Core/ModuleLog.h"
#include <Arduino.h>
#include <string.h>

static bool nameEquals(const char* a, const char* b) {
    if (!a || !b) return false;
    return strncmp(a, b, 15) == 0;
}

void ActuatorsModule::configureSlot(uint8_t idx,
                                    const char* name,
                                    uint8_t pin,
                                    ActuatorKind kind,
                                    bool momentary,
                                    uint16_t momentaryMs,
                                    uint16_t momentaryMinIntervalMs,
                                    bool activeHigh,
                                    float flowRateLh,
                                    float tankVolumeL,
                                    float tankFillPct)
{
    if (idx >= ACTUATOR_MAX) return;
    ActuatorConfig& c = cfgData[idx];
    c.enabled = true;
    c.pin = pin;
    c.kind = kind;
    c.momentary = momentary;
    c.momentaryMs = momentaryMs;
    c.momentaryMinIntervalMs = momentaryMinIntervalMs;
    c.activeHigh = activeHigh;
    c.flowRateLh = flowRateLh;
    c.tankVolumeL = tankVolumeL;
    c.tankFillPct = tankFillPct;

    if (name) {
        strncpy(c.name, name, sizeof(c.name) - 1);
        c.name[sizeof(c.name) - 1] = '\0';
    }
}

void ActuatorsModule::init(ConfigStore& cfg, I2CManager&, ServiceRegistry& services)
{
    logHub = services.get<LogHubService>("loghub");
    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore = dsSvc ? dsSvc->store : nullptr;

    // Register config vars
    cfg.registerVar(a0_en); cfg.registerVar(a0_pin); cfg.registerVar(a0_ah); cfg.registerVar(a0_mo);
    cfg.registerVar(a0_mm); cfg.registerVar(a0_mi); cfg.registerVar(a0_kind); cfg.registerVar(a0_fr);
    cfg.registerVar(a0_tv); cfg.registerVar(a0_tf); cfg.registerVar(a0_nm);

    cfg.registerVar(a1_en); cfg.registerVar(a1_pin); cfg.registerVar(a1_ah); cfg.registerVar(a1_mo);
    cfg.registerVar(a1_mm); cfg.registerVar(a1_mi); cfg.registerVar(a1_kind); cfg.registerVar(a1_fr);
    cfg.registerVar(a1_tv); cfg.registerVar(a1_tf); cfg.registerVar(a1_nm);

    cfg.registerVar(a2_en); cfg.registerVar(a2_pin); cfg.registerVar(a2_ah); cfg.registerVar(a2_mo);
    cfg.registerVar(a2_mm); cfg.registerVar(a2_mi); cfg.registerVar(a2_kind); cfg.registerVar(a2_fr);
    cfg.registerVar(a2_tv); cfg.registerVar(a2_tf); cfg.registerVar(a2_nm);

    cfg.registerVar(a3_en); cfg.registerVar(a3_pin); cfg.registerVar(a3_ah); cfg.registerVar(a3_mo);
    cfg.registerVar(a3_mm); cfg.registerVar(a3_mi); cfg.registerVar(a3_kind); cfg.registerVar(a3_fr);
    cfg.registerVar(a3_tv); cfg.registerVar(a3_tf); cfg.registerVar(a3_nm);

    cfg.registerVar(a4_en); cfg.registerVar(a4_pin); cfg.registerVar(a4_ah); cfg.registerVar(a4_mo);
    cfg.registerVar(a4_mm); cfg.registerVar(a4_mi); cfg.registerVar(a4_kind); cfg.registerVar(a4_fr);
    cfg.registerVar(a4_tv); cfg.registerVar(a4_tf); cfg.registerVar(a4_nm);

    cfg.registerVar(a5_en); cfg.registerVar(a5_pin); cfg.registerVar(a5_ah); cfg.registerVar(a5_mo);
    cfg.registerVar(a5_mm); cfg.registerVar(a5_mi); cfg.registerVar(a5_kind); cfg.registerVar(a5_fr);
    cfg.registerVar(a5_tv); cfg.registerVar(a5_tf); cfg.registerVar(a5_nm);

    for (uint8_t i = 0; i < ACTUATOR_MAX; ++i) {
        timerCtx[i] = { this, i };
        pulseTimers[i] = xTimerCreate(
            "act_pulse",
            pdMS_TO_TICKS(10),
            pdFALSE,
            &timerCtx[i],
            &ActuatorsModule::momentaryTimerCb
        );
        applyConfig(i);
        rtData[i].tankFillPct = cfgData[i].tankFillPct;
        if (dataStore) setActuatorRuntime(*dataStore, i, rtData[i]);
    }

    services.add("actuators", &svc);
    LOGI("ActuatorsService registered (%u slots)", (unsigned)ACTUATOR_MAX);
}

void ActuatorsModule::applyConfig(uint8_t idx)
{
    if (idx >= ACTUATOR_MAX) return;
    const ActuatorConfig& c = cfgData[idx];
    if (c.pin == 0) return;

    pinMode(c.pin, OUTPUT);
    bool inactive = c.activeHigh ? LOW : HIGH;
    digitalWrite(c.pin, inactive);
}

bool ActuatorsModule::svcSet(void* ctx, const char* name, bool on)
{
    ActuatorsModule* self = static_cast<ActuatorsModule*>(ctx);
    int idx = self->findIndexByName(name);
    if (idx < 0) return false;
    self->drivePin((uint8_t)idx, on);
    return true;
}

bool ActuatorsModule::svcToggle(void* ctx, const char* name)
{
    ActuatorsModule* self = static_cast<ActuatorsModule*>(ctx);
    int idx = self->findIndexByName(name);
    if (idx < 0) return false;
    bool next = !self->isOn((uint8_t)idx);
    self->drivePin((uint8_t)idx, next);
    return true;
}

bool ActuatorsModule::svcGet(void* ctx, const char* name, bool* outOn)
{
    ActuatorsModule* self = static_cast<ActuatorsModule*>(ctx);
    int idx = self->findIndexByName(name);
    if (idx < 0 || !outOn) return false;
    *outOn = self->isOn((uint8_t)idx);
    return true;
}

int ActuatorsModule::findIndexByName(const char* name) const
{
    if (!name) return -1;
    for (uint8_t i = 0; i < ACTUATOR_MAX; ++i) {
        if (nameEquals(cfgData[i].name, name)) return (int)i;
    }
    return -1;
}

void ActuatorsModule::drivePin(uint8_t idx, bool on)
{
    if (idx >= ACTUATOR_MAX) return;
    ActuatorConfig& c = cfgData[idx];
    if (!c.enabled || c.pin == 0) return;

    bool levelOn = c.activeHigh ? HIGH : LOW;
    bool levelOff = c.activeHigh ? LOW : HIGH;

    if (c.momentary) {
        if (virtualState[idx] == on) return;

        uint32_t now = millis();
        if (c.momentaryMinIntervalMs > 0) {
            if ((uint32_t)(now - lastPulseMs[idx]) < c.momentaryMinIntervalMs) return;
        }

        TimerHandle_t t = pulseTimers[idx];
        if (t && xTimerIsTimerActive(t) != pdFALSE) return;

        digitalWrite(c.pin, levelOn);
        if (t) {
            uint32_t duration = c.momentaryMs > 0 ? c.momentaryMs : 1;
            xTimerChangePeriod(t, pdMS_TO_TICKS(duration), 0);
            xTimerStart(t, 0);
        }
        lastPulseMs[idx] = now;
        virtualState[idx] = on;
    } else {
        digitalWrite(c.pin, on ? levelOn : levelOff);
    }

    rtData[idx].commanded = on;
}

bool ActuatorsModule::isOn(uint8_t idx) const
{
    if (idx >= ACTUATOR_MAX) return false;
    const ActuatorConfig& c = cfgData[idx];
    if (c.momentary) return virtualState[idx];
    if (c.pin == 0) return false;
    bool levelOn = c.activeHigh ? HIGH : LOW;
    return digitalRead(c.pin) == levelOn;
}

void ActuatorsModule::updateUptime(uint32_t now)
{
    if (lastTickMs == 0) {
        lastTickMs = now;
        return;
    }

    uint32_t dt = now - lastTickMs;
    lastTickMs = now;

    for (uint8_t i = 0; i < ACTUATOR_MAX; ++i) {
        if (!cfgData[i].enabled) continue;
        bool on = isOn(i);
        rtData[i].on = on;
        if (on) {
            rtData[i].uptimeMs += dt;
        }
    }
}

void ActuatorsModule::updateTankFill(uint8_t idx, uint32_t now)
{
    (void)now;
    ActuatorConfig& c = cfgData[idx];
    if (c.kind != ActuatorKind::Pump) return;
    if (c.tankVolumeL <= 0.0f || c.flowRateLh <= 0.0f) {
        rtData[idx].tankFillPct = c.tankFillPct;
        return;
    }

    float hours = (float)rtData[idx].uptimeMs / 3600000.0f;
    float usedL = hours * c.flowRateLh;
    float usedPct = (usedL / c.tankVolumeL) * 100.0f;
    float remaining = c.tankFillPct - usedPct;
    if (remaining < 0.0f) remaining = 0.0f;
    rtData[idx].tankFillPct = remaining;
}

void ActuatorsModule::momentaryTimerCb(TimerHandle_t t)
{
    TimerCtx* ctx = static_cast<TimerCtx*>(pvTimerGetTimerID(t));
    if (!ctx || !ctx->self) return;
    ctx->self->handleMomentaryTimeout(ctx->idx);
}

void ActuatorsModule::handleMomentaryTimeout(uint8_t idx)
{
    if (idx >= ACTUATOR_MAX) return;
    ActuatorConfig& c = cfgData[idx];
    if (c.pin != 0) {
        bool levelOff = c.activeHigh ? LOW : HIGH;
        digitalWrite(c.pin, levelOff);
    }
}

void ActuatorsModule::loop()
{
    uint32_t now = millis();

    updateUptime(now);

    for (uint8_t i = 0; i < ACTUATOR_MAX; ++i) {
        updateTankFill(i, now);
        if (dataStore) setActuatorRuntime(*dataStore, i, rtData[i]);
    }

    vTaskDelay(pdMS_TO_TICKS(200));
}
