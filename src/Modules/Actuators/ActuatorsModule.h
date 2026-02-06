#pragma once
/**
 * @file ActuatorsModule.h
 * @brief Generic actuators module.
 */
#include "Core/Module.h"
#include "Core/Services/Services.h"
#include "Modules/Actuators/ActuatorsModuleDataModel.h"

#include <stdint.h>
#include "freertos/timers.h"

enum class ActuatorKind : uint8_t {
    Relay = 0,
    Pump = 1,
    Light = 2
};

struct ActuatorConfig {
    bool enabled = false;
    uint8_t pin = 0;
    bool activeHigh = false;
    bool momentary = false;
    uint16_t momentaryMs = 400;
    uint16_t momentaryMinIntervalMs = 1000;
    ActuatorKind kind = ActuatorKind::Relay;
    float flowRateLh = 0.0f;
    float tankVolumeL = 0.0f;
    float tankFillPct = 100.0f;
    char name[16] = "";
};

struct ActuatorService {
    bool (*set)(void* ctx, const char* name, bool on);
    bool (*toggle)(void* ctx, const char* name);
    bool (*get)(void* ctx, const char* name, bool* outOn);
    void* ctx;
};

class ActuatorsModule : public Module {
public:
    const char* moduleId() const override { return "actuators"; }
    const char* taskName() const override { return "actuators"; }

    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "datastore";
        return nullptr;
    }

    void init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) override;
    void loop() override;

    void configureSlot(uint8_t idx,
                       const char* name,
                       uint8_t pin,
                       ActuatorKind kind,
                       bool momentary = false,
                       uint16_t momentaryMs = 400,
                       uint16_t momentaryMinIntervalMs = 1000,
                       bool activeHigh = false,
                       float flowRateLh = 0.0f,
                       float tankVolumeL = 0.0f,
                       float tankFillPct = 100.0f);

private:
    static bool svcSet(void* ctx, const char* name, bool on);
    static bool svcToggle(void* ctx, const char* name);
    static bool svcGet(void* ctx, const char* name, bool* outOn);

    int findIndexByName(const char* name) const;
    void applyConfig(uint8_t idx);
    void drivePin(uint8_t idx, bool on);
    void updateUptime(uint32_t now);
    void updateTankFill(uint8_t idx, uint32_t now);
    bool isOn(uint8_t idx) const;
    static void momentaryTimerCb(TimerHandle_t t);
    void handleMomentaryTimeout(uint8_t idx);

    ActuatorConfig cfgData[ACTUATOR_MAX];
    ActuatorRuntime rtData[ACTUATOR_MAX];

    bool virtualState[ACTUATOR_MAX] = {false};
    uint32_t lastTickMs = 0;
    uint32_t lastPulseMs[ACTUATOR_MAX] = {0};
    TimerHandle_t pulseTimers[ACTUATOR_MAX] = {nullptr};
    struct TimerCtx { ActuatorsModule* self; uint8_t idx; };
    TimerCtx timerCtx[ACTUATOR_MAX] = {};

    const LogHubService* logHub = nullptr;
    DataStore* dataStore = nullptr;

    ActuatorService svc { svcSet, svcToggle, svcGet, this };

    // ---- Config vars (4 slots) ----
    // Slot 0
    ConfigVariable<bool,0> a0_en {
        NVS_KEY("act0_en"),"a0_enabled","actuators",ConfigType::Bool,
        &cfgData[0].enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a0_pin {
        NVS_KEY("act0_pn"),"a0_pin","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[0].pin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a0_ah {
        NVS_KEY("act0_ah"),"a0_active_high","actuators",ConfigType::Bool,
        &cfgData[0].activeHigh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a0_mo {
        NVS_KEY("act0_mo"),"a0_momentary","actuators",ConfigType::Bool,
        &cfgData[0].momentary,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a0_mm {
        NVS_KEY("act0_mm"),"a0_momentary_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[0].momentaryMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a0_mi {
        NVS_KEY("act0_mi"),"a0_momentary_min_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[0].momentaryMinIntervalMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a0_kind {
        NVS_KEY("act0_kd"),"a0_kind","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[0].kind,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a0_fr {
        NVS_KEY("act0_fr"),"a0_flow_lh","actuators",ConfigType::Float,
        &cfgData[0].flowRateLh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a0_tv {
        NVS_KEY("act0_tv"),"a0_tank_volume","actuators",ConfigType::Float,
        &cfgData[0].tankVolumeL,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a0_tf {
        NVS_KEY("act0_tf"),"a0_tank_fill","actuators",ConfigType::Float,
        &cfgData[0].tankFillPct,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> a0_nm {
        NVS_KEY("act0_nm"),"a0_name","actuators",ConfigType::CharArray,
        (char*)cfgData[0].name,ConfigPersistence::Persistent,sizeof(cfgData[0].name)
    };

    // Slot 1
    ConfigVariable<bool,0> a1_en {
        NVS_KEY("act1_en"),"a1_enabled","actuators",ConfigType::Bool,
        &cfgData[1].enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a1_pin {
        NVS_KEY("act1_pn"),"a1_pin","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[1].pin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a1_ah {
        NVS_KEY("act1_ah"),"a1_active_high","actuators",ConfigType::Bool,
        &cfgData[1].activeHigh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a1_mo {
        NVS_KEY("act1_mo"),"a1_momentary","actuators",ConfigType::Bool,
        &cfgData[1].momentary,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a1_mm {
        NVS_KEY("act1_mm"),"a1_momentary_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[1].momentaryMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a1_mi {
        NVS_KEY("act1_mi"),"a1_momentary_min_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[1].momentaryMinIntervalMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a1_kind {
        NVS_KEY("act1_kd"),"a1_kind","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[1].kind,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a1_fr {
        NVS_KEY("act1_fr"),"a1_flow_lh","actuators",ConfigType::Float,
        &cfgData[1].flowRateLh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a1_tv {
        NVS_KEY("act1_tv"),"a1_tank_volume","actuators",ConfigType::Float,
        &cfgData[1].tankVolumeL,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a1_tf {
        NVS_KEY("act1_tf"),"a1_tank_fill","actuators",ConfigType::Float,
        &cfgData[1].tankFillPct,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> a1_nm {
        NVS_KEY("act1_nm"),"a1_name","actuators",ConfigType::CharArray,
        (char*)cfgData[1].name,ConfigPersistence::Persistent,sizeof(cfgData[1].name)
    };

    // Slot 2
    ConfigVariable<bool,0> a2_en {
        NVS_KEY("act2_en"),"a2_enabled","actuators",ConfigType::Bool,
        &cfgData[2].enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a2_pin {
        NVS_KEY("act2_pn"),"a2_pin","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[2].pin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a2_ah {
        NVS_KEY("act2_ah"),"a2_active_high","actuators",ConfigType::Bool,
        &cfgData[2].activeHigh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a2_mo {
        NVS_KEY("act2_mo"),"a2_momentary","actuators",ConfigType::Bool,
        &cfgData[2].momentary,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a2_mm {
        NVS_KEY("act2_mm"),"a2_momentary_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[2].momentaryMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a2_mi {
        NVS_KEY("act2_mi"),"a2_momentary_min_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[2].momentaryMinIntervalMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a2_kind {
        NVS_KEY("act2_kd"),"a2_kind","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[2].kind,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a2_fr {
        NVS_KEY("act2_fr"),"a2_flow_lh","actuators",ConfigType::Float,
        &cfgData[2].flowRateLh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a2_tv {
        NVS_KEY("act2_tv"),"a2_tank_volume","actuators",ConfigType::Float,
        &cfgData[2].tankVolumeL,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a2_tf {
        NVS_KEY("act2_tf"),"a2_tank_fill","actuators",ConfigType::Float,
        &cfgData[2].tankFillPct,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> a2_nm {
        NVS_KEY("act2_nm"),"a2_name","actuators",ConfigType::CharArray,
        (char*)cfgData[2].name,ConfigPersistence::Persistent,sizeof(cfgData[2].name)
    };

    // Slot 3
    ConfigVariable<bool,0> a3_en {
        NVS_KEY("act3_en"),"a3_enabled","actuators",ConfigType::Bool,
        &cfgData[3].enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a3_pin {
        NVS_KEY("act3_pn"),"a3_pin","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[3].pin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a3_ah {
        NVS_KEY("act3_ah"),"a3_active_high","actuators",ConfigType::Bool,
        &cfgData[3].activeHigh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a3_mo {
        NVS_KEY("act3_mo"),"a3_momentary","actuators",ConfigType::Bool,
        &cfgData[3].momentary,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a3_mm {
        NVS_KEY("act3_mm"),"a3_momentary_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[3].momentaryMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a3_mi {
        NVS_KEY("act3_mi"),"a3_momentary_min_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[3].momentaryMinIntervalMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a3_kind {
        NVS_KEY("act3_kd"),"a3_kind","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[3].kind,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a3_fr {
        NVS_KEY("act3_fr"),"a3_flow_lh","actuators",ConfigType::Float,
        &cfgData[3].flowRateLh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a3_tv {
        NVS_KEY("act3_tv"),"a3_tank_volume","actuators",ConfigType::Float,
        &cfgData[3].tankVolumeL,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a3_tf {
        NVS_KEY("act3_tf"),"a3_tank_fill","actuators",ConfigType::Float,
        &cfgData[3].tankFillPct,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> a3_nm {
        NVS_KEY("act3_nm"),"a3_name","actuators",ConfigType::CharArray,
        (char*)cfgData[3].name,ConfigPersistence::Persistent,sizeof(cfgData[3].name)
    };

    // Slot 4
    ConfigVariable<bool,0> a4_en {
        NVS_KEY("act4_en"),"a4_enabled","actuators",ConfigType::Bool,
        &cfgData[4].enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a4_pin {
        NVS_KEY("act4_pn"),"a4_pin","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[4].pin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a4_ah {
        NVS_KEY("act4_ah"),"a4_active_high","actuators",ConfigType::Bool,
        &cfgData[4].activeHigh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a4_mo {
        NVS_KEY("act4_mo"),"a4_momentary","actuators",ConfigType::Bool,
        &cfgData[4].momentary,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a4_mm {
        NVS_KEY("act4_mm"),"a4_momentary_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[4].momentaryMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a4_mi {
        NVS_KEY("act4_mi"),"a4_momentary_min_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[4].momentaryMinIntervalMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a4_kind {
        NVS_KEY("act4_kd"),"a4_kind","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[4].kind,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a4_fr {
        NVS_KEY("act4_fr"),"a4_flow_lh","actuators",ConfigType::Float,
        &cfgData[4].flowRateLh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a4_tv {
        NVS_KEY("act4_tv"),"a4_tank_volume","actuators",ConfigType::Float,
        &cfgData[4].tankVolumeL,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a4_tf {
        NVS_KEY("act4_tf"),"a4_tank_fill","actuators",ConfigType::Float,
        &cfgData[4].tankFillPct,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> a4_nm {
        NVS_KEY("act4_nm"),"a4_name","actuators",ConfigType::CharArray,
        (char*)cfgData[4].name,ConfigPersistence::Persistent,sizeof(cfgData[4].name)
    };

    // Slot 5
    ConfigVariable<bool,0> a5_en {
        NVS_KEY("act5_en"),"a5_enabled","actuators",ConfigType::Bool,
        &cfgData[5].enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a5_pin {
        NVS_KEY("act5_pn"),"a5_pin","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[5].pin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a5_ah {
        NVS_KEY("act5_ah"),"a5_active_high","actuators",ConfigType::Bool,
        &cfgData[5].activeHigh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> a5_mo {
        NVS_KEY("act5_mo"),"a5_momentary","actuators",ConfigType::Bool,
        &cfgData[5].momentary,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a5_mm {
        NVS_KEY("act5_mm"),"a5_momentary_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[5].momentaryMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a5_mi {
        NVS_KEY("act5_mi"),"a5_momentary_min_ms","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[5].momentaryMinIntervalMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> a5_kind {
        NVS_KEY("act5_kd"),"a5_kind","actuators",ConfigType::Int32,
        (int32_t*)&cfgData[5].kind,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a5_fr {
        NVS_KEY("act5_fr"),"a5_flow_lh","actuators",ConfigType::Float,
        &cfgData[5].flowRateLh,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a5_tv {
        NVS_KEY("act5_tv"),"a5_tank_volume","actuators",ConfigType::Float,
        &cfgData[5].tankVolumeL,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> a5_tf {
        NVS_KEY("act5_tf"),"a5_tank_fill","actuators",ConfigType::Float,
        &cfgData[5].tankFillPct,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> a5_nm {
        NVS_KEY("act5_nm"),"a5_name","actuators",ConfigType::CharArray,
        (char*)cfgData[5].name,ConfigPersistence::Persistent,sizeof(cfgData[5].name)
    };
};
