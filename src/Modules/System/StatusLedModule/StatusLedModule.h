#pragma once
/**
 * @file StatusLedModule.h
 * @brief RGB status LED driver for the ESP32-S3 built-in NeoPixel (GPIO 48).
 */

#include "Core/Module.h"
#include "Core/Services/Services.h"

class DataStore;

/**
 * @brief Drives the GPIO-48 WS2812 LED with a priority-based status machine.
 *
 * State priority (highest first):
 *   Ota > AlarmCritical > AlarmOther > ProvisioningAp > NoWifi > NoMqtt > Nominal
 * The initial state is Boot (white breathing) until WiFi connects for the first time.
 *
 * FirmwareUpdate and NetworkAccess services are resolved in onStart() so that
 * they remain optional (no hard module dependency).
 */
class StatusLedModule : public Module {
public:
    ModuleId moduleId() const override { return ModuleId::StatusLed; }
    const char* taskName() const override { return "status_led"; }
    BaseType_t taskCore() const override { return 1; }
    uint16_t taskStackSize() const override { return 1536; }
    UBaseType_t taskPriority() const override { return 1; }
    uint8_t taskCount() const override { return 1; }
    const ModuleTaskSpec* taskSpecs() const override { return singleLoopTaskSpec(); }

    uint8_t dependencyCount() const override { return 2; }
    ModuleId dependency(uint8_t i) const override {
        if (i == 0) return ModuleId::DataStore;
        if (i == 1) return ModuleId::Alarm;
        return ModuleId::Unknown;
    }

    void init(ConfigStore& cfg, ServiceRegistry& services) override;
    void onStart(ConfigStore& cfg, ServiceRegistry& services) override;
    void loop() override;

private:
    enum class LedState : uint8_t {
        Boot = 0,
        Ota,
        AlarmCritical,
        AlarmOther,
        ProvisioningAp,
        NoWifi,
        NoMqtt,
        Nominal,
        _Count
    };

    enum class Pattern : uint8_t {
        Solid,
        BlinkFast,  // 100 ms on / 100 ms off
        BlinkSlow,  // 500 ms on / 500 ms off
        Breathe,    // linear triangle, period 2000 ms
    };

    struct Rgb { uint8_t r, g, b; };
    struct StateSpec { Rgb color; Pattern pattern; };

    static constexpr uint8_t kPin  = 48;
    static constexpr uint8_t kBr   = 25;   // max brightness (0-255)

    static const StateSpec kSpecs[static_cast<uint8_t>(LedState::_Count)];

    LedState evaluateState_() const;
    void applyPattern_(const StateSpec& spec, uint32_t nowMs);
    void write_(uint8_t r, uint8_t g, uint8_t b);

    DataStore*                       ds_        = nullptr;
    const AlarmService*              alarmSvc_  = nullptr;
    const FirmwareUpdateService*     fwSvc_     = nullptr;
    const NetworkAccessService*      netSvc_    = nullptr;

    bool hadWifi_ = false;
};
