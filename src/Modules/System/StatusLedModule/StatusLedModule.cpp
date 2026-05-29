#include "Modules/System/StatusLedModule/StatusLedModule.h"

#include <Arduino.h>

#include "Core/DataStore/DataStore.h"

// ---------------------------------------------------------------------------
// State → (color, pattern) table — indexed by LedState enum value
// ---------------------------------------------------------------------------
const StatusLedModule::StateSpec StatusLedModule::kSpecs[] = {
    //  color {r,  g,  b}   pattern
    {{ 25,  25,  25 }, Pattern::Breathe   }, // Boot          — blanc breathing
    {{  0,  25,  25 }, Pattern::BlinkFast }, // Ota           — cyan blink rapide
    {{ 25,   0,   0 }, Pattern::BlinkFast }, // AlarmCritical — rouge blink rapide
    {{ 25,  12,   0 }, Pattern::BlinkSlow }, // AlarmOther    — orange blink lent
    {{  0,   0,  25 }, Pattern::BlinkSlow }, // ProvisioningAp— bleu blink lent
    {{ 25,  20,   0 }, Pattern::BlinkFast }, // NoWifi        — jaune blink rapide
    {{ 25,  20,   0 }, Pattern::BlinkSlow }, // NoMqtt        — jaune blink lent
    {{  0,  20,   0 }, Pattern::Solid     }, // Nominal       — vert fixe
};

// ---------------------------------------------------------------------------

void StatusLedModule::init(ConfigStore&, ServiceRegistry& services)
{
    auto* dsSvc = services.get<DataStoreService>(ServiceId::DataStore);
    ds_         = dsSvc ? dsSvc->store : nullptr;
    alarmSvc_   = services.get<AlarmService>(ServiceId::Alarm);

    // Show white immediately so the LED is active from the very first tick
    write_(kBr, kBr, kBr);
}

void StatusLedModule::onStart(ConfigStore&, ServiceRegistry& services)
{
    // Resolved after all modules have run init(), so these are optional
    fwSvc_  = services.get<FirmwareUpdateService>(ServiceId::FirmwareUpdate);
    netSvc_ = services.get<NetworkAccessService>(ServiceId::NetworkAccess);
}

void StatusLedModule::loop()
{
    const uint32_t now = millis();
    if (ds_ && ds_->data().wifi.ready) hadWifi_ = true;
    applyPattern_(kSpecs[static_cast<uint8_t>(evaluateState_())], now);
}

// ---------------------------------------------------------------------------

StatusLedModule::LedState StatusLedModule::evaluateState_() const
{
    if (fwSvc_ && fwSvc_->isBusy(fwSvc_->ctx))
        return LedState::Ota;

    if (alarmSvc_ && alarmSvc_->activeCount(alarmSvc_->ctx) > 0) {
        const AlarmSeverity sev = alarmSvc_->highestSeverity(alarmSvc_->ctx);
        if (sev >= AlarmSeverity::Critical) return LedState::AlarmCritical;
        if (sev >= AlarmSeverity::Warning)  return LedState::AlarmOther;
    }

    if (netSvc_ && netSvc_->mode(netSvc_->ctx) == NetworkAccessMode::AccessPoint)
        return LedState::ProvisioningAp;

    if (!ds_) return LedState::Boot;

    const auto& rt = ds_->data();
    if (!rt.wifi.ready)    return hadWifi_ ? LedState::NoWifi : LedState::Boot;
    if (!rt.mqtt.mqttReady) return LedState::NoMqtt;

    return LedState::Nominal;
}

void StatusLedModule::applyPattern_(const StateSpec& spec, uint32_t nowMs)
{
    uint8_t r = spec.color.r;
    uint8_t g = spec.color.g;
    uint8_t b = spec.color.b;

    switch (spec.pattern) {
        case Pattern::Solid:
            break;

        case Pattern::BlinkFast:
            if ((nowMs % 200U) >= 100U) { r = g = b = 0; }
            break;

        case Pattern::BlinkSlow:
            if ((nowMs % 1000U) >= 500U) { r = g = b = 0; }
            break;

        case Pattern::Breathe: {
            // Linear triangle, period 2000 ms, offset by 1000 ms so we start at peak brightness
            const uint32_t phase = (nowMs + 1000U) % 2000U;
            const uint8_t scale  = (phase < 1000U)
                ? (uint8_t)(phase * 255U / 1000U)
                : (uint8_t)((2000U - phase) * 255U / 1000U);
            r = (uint8_t)((uint16_t)r * scale / 255U);
            g = (uint8_t)((uint16_t)g * scale / 255U);
            b = (uint8_t)((uint16_t)b * scale / 255U);
            break;
        }
    }

    write_(r, g, b);
}

void StatusLedModule::write_(uint8_t r, uint8_t g, uint8_t b)
{
    neopixelWrite(kPin, r, g, b);
}
