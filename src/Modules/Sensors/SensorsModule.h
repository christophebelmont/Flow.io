#pragma once
/**
 * @file SensorsModule.h
 * @brief Sensor orchestration module.
 */
#include "Core/Module.h"
#include "Core/Services/Services.h"

#include "Modules/Sensors/Bus/OneWireBus.h"
#include "Modules/Sensors/Drivers/ActuatorVirtualDriver.h"
#include "Modules/Sensors/Drivers/DallasTempDriver.h"
#include "Modules/Sensors/Engine/CachedSensor.h"
#include "Modules/Sensors/Engine/SensorRegistry.h"
#include "Modules/Sensors/Engine/SensorPipeline.h"
#include "Modules/Sensors/Filters/InvalidValueFilter.h"
#include "Modules/Sensors/Filters/RangeFilter.h"
#include "Modules/Sensors/Filters/RunningMedianAverageFilter.h"
#include <ADS1X15.h>
#include "freertos/semphr.h"

/** @brief Sensor configuration values. */
struct SensorsConfig {
    bool enabled = true;
    int32_t onewirePinWater = 19;
    int32_t onewirePinAir = 18;
    int32_t adcMode = 0; // 0=internal (ph/orp on primary), 1=external (ph/orp on secondary)
    int32_t pollMs = 1000;
    int32_t adcPollMs = 125;
    int32_t i2cSda = 21;
    int32_t i2cScl = 22;
    uint8_t adsAddr = 0x48;
    uint8_t adsAddr2 = 0x49;
    int32_t adsGain = ADS1X15_GAIN_6144MV;
    int32_t adsRate = 1; // 16 sps for ADS1115
    float adsMin = -32768.0f;
    float adsMax = 32767.0f;
    float phC0 = 1.0f;
    float phC1 = 0.0f;
    int32_t phPrec = 1;
    float orpC0 = 1.0f;
    float orpC1 = 0.0f;
    int32_t orpPrec = 0;
    float psiC0 = 1.0f;
    float psiC1 = 0.0f;
    int32_t psiPrec = 1;
    float waterTempC0 = 1.0f;
    float waterTempC1 = 0.0f;
    int32_t waterTempPrec = 1;
    float airTempC0 = 1.0f;
    float airTempC1 = 0.0f;
    int32_t airTempPrec = 1;
    bool virtualActuators = true;
};

class SensorsModule : public Module {
public:
    const char* moduleId() const override { return "sensors"; }
    const char* taskName() const override { return "sensors"; }

    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "datastore";
        return nullptr;
    }

    void init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) override;
    void loop() override;

    void setOneWireBuses(OneWireBus* water, OneWireBus* air) {
        oneWireWater = water;
        oneWireAir = air;
    }
    void setAdsDevices(ADS1115* primary, ADS1115* secondary) {
        adsPrimary = primary;
        adsSecondary = secondary;
        useExternalPhOrpOverride = false;
    }
    void setPhOrpExternalOverride(bool enabled) {
        useExternalPhOrpOverride = true;
        useExternalPhOrp = enabled;
    }

private:
    void setupDallasAddresses();
    void setupSensors();
    void adcLoop();
    static void adcTaskEntry(void* arg);
    void lockI2C();
    void unlockI2C();
    void applyAdcSample(CachedSensor& target, float value, bool valid, SensorPipeline* pipeline);
    float applyCalibration(float value, float c0, float c1) const { return c0 * value + c1; }
    float applyPrecision(float value, int32_t decimals) const;
    float adsToMilliVolts(ADS1115* ads, int16_t raw) const;

    SensorsConfig cfgData;

    const LogHubService* logHub = nullptr;
    DataStore* dataStore = nullptr;

    ConfigVariable<bool,0> enabledVar {
        NVS_KEY("sens_en"),"enabled","sensors",ConfigType::Bool,
        &cfgData.enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> oneWirePinVar {
        NVS_KEY("sens_oww"),"onewire_water_pin","sensors",ConfigType::Int32,
        &cfgData.onewirePinWater,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> oneWireAirPinVar {
        NVS_KEY("sens_owa"),"onewire_air_pin","sensors",ConfigType::Int32,
        &cfgData.onewirePinAir,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adcModeVar {
        NVS_KEY("sens_mode"),"adc_mode","sensors",ConfigType::Int32,
        &cfgData.adcMode,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> pollMsVar {
        NVS_KEY("sens_poll"),"poll_ms","sensors",ConfigType::Int32,
        &cfgData.pollMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adcPollMsVar {
        NVS_KEY("sens_apol"),"adc_poll_ms","sensors",ConfigType::Int32,
        &cfgData.adcPollMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> i2cSdaVar {
        NVS_KEY("sens_sda"),"i2c_sda","sensors",ConfigType::Int32,
        &cfgData.i2cSda,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> i2cSclVar {
        NVS_KEY("sens_scl"),"i2c_scl","sensors",ConfigType::Int32,
        &cfgData.i2cScl,ConfigPersistence::Persistent,0
    };
    ConfigVariable<uint8_t,0> adsAddrVar {
        NVS_KEY("sens_ads"),"ads_addr","sensors",ConfigType::UInt8,
        &cfgData.adsAddr,ConfigPersistence::Persistent,0
    };
    ConfigVariable<uint8_t,0> adsAddr2Var {
        NVS_KEY("sens_ad2"),"ads_addr2","sensors",ConfigType::UInt8,
        &cfgData.adsAddr2,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adsGainVar {
        NVS_KEY("sens_gain"),"ads_gain","sensors",ConfigType::Int32,
        &cfgData.adsGain,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adsRateVar {
        NVS_KEY("sens_rate"),"ads_rate","sensors",ConfigType::Int32,
        &cfgData.adsRate,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> adsMinVar {
        NVS_KEY("sens_adn"),"ads_min","sensors",ConfigType::Float,
        &cfgData.adsMin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> adsMaxVar {
        NVS_KEY("sens_adx"),"ads_max","sensors",ConfigType::Float,
        &cfgData.adsMax,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> phC0Var {
        NVS_KEY("sens_ph0"),"ph_c0","sensors",ConfigType::Float,
        &cfgData.phC0,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> phC1Var {
        NVS_KEY("sens_ph1"),"ph_c1","sensors",ConfigType::Float,
        &cfgData.phC1,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> phPrecVar {
        NVS_KEY("sens_php"),"ph_prec","sensors",ConfigType::Int32,
        &cfgData.phPrec,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> orpC0Var {
        NVS_KEY("sens_or0"),"orp_c0","sensors",ConfigType::Float,
        &cfgData.orpC0,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> orpC1Var {
        NVS_KEY("sens_or1"),"orp_c1","sensors",ConfigType::Float,
        &cfgData.orpC1,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> orpPrecVar {
        NVS_KEY("sens_orp"),"orp_prec","sensors",ConfigType::Int32,
        &cfgData.orpPrec,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> psiC0Var {
        NVS_KEY("sens_ps0"),"psi_c0","sensors",ConfigType::Float,
        &cfgData.psiC0,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> psiC1Var {
        NVS_KEY("sens_ps1"),"psi_c1","sensors",ConfigType::Float,
        &cfgData.psiC1,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> psiPrecVar {
        NVS_KEY("sens_psp"),"psi_prec","sensors",ConfigType::Int32,
        &cfgData.psiPrec,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> waterC0Var {
        NVS_KEY("sens_tw0"),"water_c0","sensors",ConfigType::Float,
        &cfgData.waterTempC0,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> waterC1Var {
        NVS_KEY("sens_tw1"),"water_c1","sensors",ConfigType::Float,
        &cfgData.waterTempC1,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> waterPrecVar {
        NVS_KEY("sens_twp"),"water_prec","sensors",ConfigType::Int32,
        &cfgData.waterTempPrec,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> airC0Var {
        NVS_KEY("sens_ta0"),"air_c0","sensors",ConfigType::Float,
        &cfgData.airTempC0,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> airC1Var {
        NVS_KEY("sens_ta1"),"air_c1","sensors",ConfigType::Float,
        &cfgData.airTempC1,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> airPrecVar {
        NVS_KEY("sens_tap"),"air_prec","sensors",ConfigType::Int32,
        &cfgData.airTempPrec,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> virtualActuatorsVar {
        NVS_KEY("sens_vac"),"virtual_actuators","sensors",ConfigType::Bool,
        &cfgData.virtualActuators,ConfigPersistence::Persistent,0
    };

    OneWireBus* oneWireWater = nullptr;
    OneWireBus* oneWireAir = nullptr;
    ADS1115* adsPrimary = nullptr;
    ADS1115* adsSecondary = nullptr;
    bool useExternalPhOrp = false;
    bool useExternalPhOrpOverride = false;
    bool adsPrimaryOk = false;
    bool adsSecondaryOk = false;
    uint32_t lastPollMs = 0;
    bool tempPrimed = false;
    uint32_t lastAdcLogMs = 0;
    SemaphoreHandle_t i2cMutex = nullptr;
    TaskHandle_t adcTaskHandle = nullptr;
    uint8_t adcChannelIndex = 0;
    bool adcRequested = false;
    uint8_t adcSecondaryIndex = 0;
    bool adcSecondaryRequested = false;

    uint8_t waterAddr[8] = {0};
    uint8_t airAddr[8] = {0};

    CachedSensor phRaw {"ph_raw"};
    CachedSensor orpRaw {"orp_raw"};
    CachedSensor pumpRaw {"pump_raw"};
    DallasTempDriver* waterTempDriver = nullptr;
    DallasTempDriver* airTempDriver = nullptr;

    InvalidValueFilter invalidFilter;
    RangeFilter adcRangeFilter {0.0f, 4095.0f};
    RangeFilter tempRangeFilter {-55.0f, 125.0f};

    RunningMedianAverageFilter phMedian {11, 5};
    RunningMedianAverageFilter orpMedian {11, 5};
    RunningMedianAverageFilter pumpMedian {11, 5};
    RunningMedianAverageFilter waterMedian {11, 5};
    RunningMedianAverageFilter airMedian {11, 5};

    ISensorFilter* phFilters[3] = { &invalidFilter, &adcRangeFilter, &phMedian };
    ISensorFilter* orpFilters[3] = { &invalidFilter, &adcRangeFilter, &orpMedian };
    ISensorFilter* pumpFilters[3] = { &invalidFilter, &adcRangeFilter, &pumpMedian };
    ISensorFilter* waterFilters[3] = { &invalidFilter, &tempRangeFilter, &waterMedian };
    ISensorFilter* airFilters[3] = { &invalidFilter, &tempRangeFilter, &airMedian };

    SensorPipeline* phSensor = nullptr;
    SensorPipeline* orpSensor = nullptr;
    SensorPipeline* pumpSensor = nullptr;
    SensorPipeline* waterSensor = nullptr;
    SensorPipeline* airSensor = nullptr;

    ActuatorVirtualDriver* actTankDrivers[ACTUATOR_MAX] = {nullptr};
    ActuatorVirtualDriver* actUptimeDrivers[ACTUATOR_MAX] = {nullptr};
    SensorPipeline* actTankSensors[ACTUATOR_MAX] = {nullptr};
    SensorPipeline* actUptimeSensors[ACTUATOR_MAX] = {nullptr};
    char actTankNames[ACTUATOR_MAX][20] = {{0}};
    char actUptimeNames[ACTUATOR_MAX][20] = {{0}};

    SensorRegistry registry;
};
