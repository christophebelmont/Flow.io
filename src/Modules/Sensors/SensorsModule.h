#pragma once
/**
 * @file SensorsModule.h
 * @brief Sensor orchestration module.
 */
#include "Core/Module.h"
#include "Core/Services/Services.h"

#include "Modules/Sensors/Bus/AdcBus.h"
#include "Modules/Sensors/Bus/OneWireBus.h"
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
    int32_t onewirePin = 4;
    int32_t adcMode = 0; // 0=internal, 1=external ADS1115
    int32_t adcPhPin = 34;
    int32_t adcOrpPin = 35;
    int32_t adcPumpPin = 32;
    int32_t pollMs = 1000;
    int32_t adcPollMs = 125;
    int32_t adcResolutionBits = 12;
    int32_t adcAtten = 3; // ADC_11db
    float adcMin = 0.0f;
    float adcMax = 4095.0f;
    uint8_t adsAddr = 0x48;
    int32_t adsGain = ADS1X15_GAIN_6144MV;
    int32_t adsRate = 1; // 16 sps for ADS1115
    float adsMin = -32768.0f;
    float adsMax = 32767.0f;
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

private:
    void setupDallasAddresses();
    void setupSensors();
    void adcLoop();
    static void adcTaskEntry(void* arg);
    void lockI2C();
    void unlockI2C();
    void applyAdcSample(CachedSensor& target, float value, bool valid, SensorPipeline* pipeline);

    SensorsConfig cfgData;

    const LogHubService* logHub = nullptr;
    DataStore* dataStore = nullptr;

    ConfigVariable<bool,0> enabledVar {
        NVS_KEY("sens_en"),"enabled","sensors",ConfigType::Bool,
        &cfgData.enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> oneWirePinVar {
        NVS_KEY("sens_ow"),"onewire_pin","sensors",ConfigType::Int32,
        &cfgData.onewirePin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adcPhPinVar {
        NVS_KEY("sens_ph"),"adc_ph_pin","sensors",ConfigType::Int32,
        &cfgData.adcPhPin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adcOrpPinVar {
        NVS_KEY("sens_orp"),"adc_orp_pin","sensors",ConfigType::Int32,
        &cfgData.adcOrpPin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adcPumpPinVar {
        NVS_KEY("sens_pmp"),"adc_pump_pin","sensors",ConfigType::Int32,
        &cfgData.adcPumpPin,ConfigPersistence::Persistent,0
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
    ConfigVariable<int32_t,0> adcResVar {
        NVS_KEY("sens_res"),"adc_res_bits","sensors",ConfigType::Int32,
        &cfgData.adcResolutionBits,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adcAttenVar {
        NVS_KEY("sens_att"),"adc_atten","sensors",ConfigType::Int32,
        &cfgData.adcAtten,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> adcMinVar {
        NVS_KEY("sens_min"),"adc_min","sensors",ConfigType::Float,
        &cfgData.adcMin,ConfigPersistence::Persistent,0
    };
    ConfigVariable<float,0> adcMaxVar {
        NVS_KEY("sens_max"),"adc_max","sensors",ConfigType::Float,
        &cfgData.adcMax,ConfigPersistence::Persistent,0
    };
    ConfigVariable<uint8_t,0> adsAddrVar {
        NVS_KEY("sens_ads"),"ads_addr","sensors",ConfigType::UInt8,
        &cfgData.adsAddr,ConfigPersistence::Persistent,0
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

    AdcBus adcBus;
    OneWireBus* oneWireBus = nullptr;
    ADS1115* ads = nullptr;
    bool useExternalAdc = false;
    uint32_t lastPollMs = 0;
    bool tempPrimed = false;
    uint32_t lastAdcLogMs = 0;
    SemaphoreHandle_t i2cMutex = nullptr;
    TaskHandle_t adcTaskHandle = nullptr;
    uint8_t adcChannelIndex = 0;
    bool adcRequested = false;

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

    SensorRegistry registry;
};
