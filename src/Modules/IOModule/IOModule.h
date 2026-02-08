#pragma once
/**
 * @file IOModule.h
 * @brief Unified IO module with endpoint registry and scheduler.
 */

#include "Core/Module.h"
#include "Core/Services/Services.h"
#include "Modules/IOModule/IOBus/I2CBus.h"
#include "Modules/IOModule/IOBus/OneWireBus.h"
#include "Modules/IOModule/IODrivers/Ads1115Driver.h"
#include "Modules/IOModule/IODrivers/Ds18b20Driver.h"
#include "Modules/IOModule/IODrivers/Pcf8574Driver.h"
#include "Modules/IOModule/IOEndpoints/AnalogSensorEndpoint.h"
#include "Modules/IOModule/IOEndpoints/Pcf8574MaskEndpoint.h"
#include "Modules/IOModule/IOEndpoints/RunningMedianAverageFloat.h"
#include "Modules/IOModule/IORegistry/IORegistry.h"
#include "Modules/IOModule/IOScheduler/IOScheduler.h"
#include "Modules/IOModule/IOModuleDataModel.h"

struct IOModuleConfig {
    bool enabled = true;
    int32_t i2cSda = 21;
    int32_t i2cScl = 22;
    int32_t adsPollMs = 125;
    int32_t dsPollMs = 2000;
    uint8_t adsInternalAddr = 0x48;
    uint8_t adsExternalAddr = 0x49;
    int32_t adsGain = ADS1X15_GAIN_6144MV;
    int32_t adsRate = 1;
    bool pcfEnabled = false;
    uint8_t pcfAddress = 0x20;
    uint8_t pcfMaskDefault = 0;
};

enum IOAnalogSource : uint8_t {
    IO_SRC_ADS_INTERNAL_SINGLE = 0,
    IO_SRC_ADS_EXTERNAL_DIFF = 1,
    IO_SRC_DS18_WATER = 2,
    IO_SRC_DS18_AIR = 3
};

struct IOAnalogDefinition {
    char id[24] = {0};
    uint8_t source = IO_SRC_ADS_INTERNAL_SINGLE;
    uint8_t channel = 0;
    float c0 = 1.0f;
    float c1 = 0.0f;
    int32_t precision = 1;
    float minValid = -32768.0f;
    float maxValid = 32767.0f;
};

struct IOAnalogProfileConfig {
    uint8_t source = IO_SRC_ADS_INTERNAL_SINGLE;
    uint8_t channel = 0;
    float c0 = 1.0f;
    float c1 = 0.0f;
    int32_t precision = 1;
    float minValid = -32768.0f;
    float maxValid = 32767.0f;
};

struct IOLedMaskService {
    bool (*setMask)(void* ctx, uint8_t mask);
    bool (*turnOn)(void* ctx, uint8_t bit);
    bool (*turnOff)(void* ctx, uint8_t bit);
    bool (*getMask)(void* ctx, uint8_t* mask);
    void* ctx;
};

class IOModule : public Module {
public:
    const char* moduleId() const override { return "io"; }
    const char* taskName() const override { return "io"; }

    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "datastore";
        return nullptr;
    }

    void init(ConfigStore& cfg, ServiceRegistry& services) override;
    void loop() override;

    void setOneWireBuses(OneWireBus* water, OneWireBus* air);
    bool defineAnalogInput(const IOAnalogDefinition& def);

    IORegistry& registry() { return registry_; }

private:
    static bool tickFastAds_(void* ctx, uint32_t nowMs);
    static bool tickSlowDs_(void* ctx, uint32_t nowMs);

    static bool svcSetMask_(void* ctx, uint8_t mask);
    static bool svcTurnOn_(void* ctx, uint8_t bit);
    static bool svcTurnOff_(void* ctx, uint8_t bit);
    static bool svcGetMask_(void* ctx, uint8_t* mask);

    bool setLedMask_(uint8_t mask, uint32_t tsMs);
    bool turnLedOn_(uint8_t bit, uint32_t tsMs);
    bool turnLedOff_(uint8_t bit, uint32_t tsMs);
    bool getLedMask_(uint8_t& mask) const;

    bool configureRuntime_();
    bool processAnalogDefinition_(uint8_t idx, uint32_t nowMs);
    void publishCanonical_(const char* id, float value);
    bool applyProfileToDefinition_(IOAnalogDefinition& def) const;
    void syncProfileDefaultsFromDefinition_(const IOAnalogDefinition& def);

    static constexpr uint8_t MAX_ANALOG_ENDPOINTS = 12;

    struct AnalogSlot {
        bool used = false;
        IOAnalogDefinition def{};
        AnalogSensorEndpoint* endpoint = nullptr;
        RunningMedianAverageFloat median{11, 5};
        bool lastRoundedValid = false;
        float lastRounded = 0.0f;
    };

    IOModuleConfig cfgData_{};
    IOAnalogProfileConfig phCfg_{};
    IOAnalogProfileConfig orpCfg_{};
    IOAnalogProfileConfig psiCfg_{};
    IOAnalogProfileConfig waterCfg_{};
    IOAnalogProfileConfig airCfg_{};

    const LogHubService* logHub_ = nullptr;
    DataStore* dataStore_ = nullptr;
    ServiceRegistry* services_ = nullptr;

    IORegistry registry_{};
    IOScheduler scheduler_{};
    I2CBus i2cBus_{};

    OneWireBus* oneWireWater_ = nullptr;
    OneWireBus* oneWireAir_ = nullptr;
    uint8_t oneWireWaterAddr_[8] = {0};
    uint8_t oneWireAirAddr_[8] = {0};

    Ads1115Driver* adsInternal_ = nullptr;
    Ads1115Driver* adsExternal_ = nullptr;
    Ds18b20Driver* dsWater_ = nullptr;
    Ds18b20Driver* dsAir_ = nullptr;

    Pcf8574Driver* pcf_ = nullptr;
    Pcf8574MaskEndpoint* ledMaskEp_ = nullptr;
    IOLedMaskService ledSvc_{ svcSetMask_, svcTurnOn_, svcTurnOff_, svcGetMask_, this };

    AnalogSlot analogSlots_[MAX_ANALOG_ENDPOINTS]{};
    bool runtimeReady_ = false;

    ConfigVariable<bool,0> enabledVar_ {
        NVS_KEY("io_en"),"enabled","io",ConfigType::Bool,
        &cfgData_.enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> i2cSdaVar_ {
        NVS_KEY("io_sda"),"i2c_sda","io",ConfigType::Int32,
        &cfgData_.i2cSda,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> i2cSclVar_ {
        NVS_KEY("io_scl"),"i2c_scl","io",ConfigType::Int32,
        &cfgData_.i2cScl,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adsPollVar_ {
        NVS_KEY("io_ads"),"ads_poll_ms","io",ConfigType::Int32,
        &cfgData_.adsPollMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> dsPollVar_ {
        NVS_KEY("io_ds"),"ds_poll_ms","io",ConfigType::Int32,
        &cfgData_.dsPollMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<uint8_t,0> adsInternalAddrVar_ {
        NVS_KEY("io_aiad"),"ads_internal_addr","io",ConfigType::UInt8,
        &cfgData_.adsInternalAddr,ConfigPersistence::Persistent,0
    };
    ConfigVariable<uint8_t,0> adsExternalAddrVar_ {
        NVS_KEY("io_aead"),"ads_external_addr","io",ConfigType::UInt8,
        &cfgData_.adsExternalAddr,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adsGainVar_ {
        NVS_KEY("io_agai"),"ads_gain","io",ConfigType::Int32,
        &cfgData_.adsGain,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> adsRateVar_ {
        NVS_KEY("io_arat"),"ads_rate","io",ConfigType::Int32,
        &cfgData_.adsRate,ConfigPersistence::Persistent,0
    };
    ConfigVariable<bool,0> pcfEnabledVar_ {
        NVS_KEY("io_pcfen"),"pcf_enabled","io",ConfigType::Bool,
        &cfgData_.pcfEnabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<uint8_t,0> pcfAddressVar_ {
        NVS_KEY("io_pcfad"),"pcf_address","io",ConfigType::UInt8,
        &cfgData_.pcfAddress,ConfigPersistence::Persistent,0
    };
    ConfigVariable<uint8_t,0> pcfMaskDefaultVar_ {
        NVS_KEY("io_pcfmk"),"pcf_mask_default","io",ConfigType::UInt8,
        &cfgData_.pcfMaskDefault,ConfigPersistence::Persistent,0
    };

    ConfigVariable<uint8_t,0> phSourceVar_{NVS_KEY("io_phs"),"ph_source","io",ConfigType::UInt8,&phCfg_.source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> phChannelVar_{NVS_KEY("io_phc"),"ph_channel","io",ConfigType::UInt8,&phCfg_.channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> phC0Var_{NVS_KEY("io_ph0"),"ph_c0","io",ConfigType::Float,&phCfg_.c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> phC1Var_{NVS_KEY("io_ph1"),"ph_c1","io",ConfigType::Float,&phCfg_.c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> phPrecVar_{NVS_KEY("io_php"),"ph_prec","io",ConfigType::Int32,&phCfg_.precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> phMinVar_{NVS_KEY("io_phn"),"ph_min","io",ConfigType::Float,&phCfg_.minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> phMaxVar_{NVS_KEY("io_phx"),"ph_max","io",ConfigType::Float,&phCfg_.maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> orpSourceVar_{NVS_KEY("io_ors"),"orp_source","io",ConfigType::UInt8,&orpCfg_.source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> orpChannelVar_{NVS_KEY("io_orc"),"orp_channel","io",ConfigType::UInt8,&orpCfg_.channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> orpC0Var_{NVS_KEY("io_or0"),"orp_c0","io",ConfigType::Float,&orpCfg_.c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> orpC1Var_{NVS_KEY("io_or1"),"orp_c1","io",ConfigType::Float,&orpCfg_.c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> orpPrecVar_{NVS_KEY("io_orp"),"orp_prec","io",ConfigType::Int32,&orpCfg_.precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> orpMinVar_{NVS_KEY("io_orn"),"orp_min","io",ConfigType::Float,&orpCfg_.minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> orpMaxVar_{NVS_KEY("io_orx"),"orp_max","io",ConfigType::Float,&orpCfg_.maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> psiSourceVar_{NVS_KEY("io_pss"),"psi_source","io",ConfigType::UInt8,&psiCfg_.source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> psiChannelVar_{NVS_KEY("io_psc"),"psi_channel","io",ConfigType::UInt8,&psiCfg_.channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> psiC0Var_{NVS_KEY("io_ps0"),"psi_c0","io",ConfigType::Float,&psiCfg_.c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> psiC1Var_{NVS_KEY("io_ps1"),"psi_c1","io",ConfigType::Float,&psiCfg_.c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> psiPrecVar_{NVS_KEY("io_psp"),"psi_prec","io",ConfigType::Int32,&psiCfg_.precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> psiMinVar_{NVS_KEY("io_psn"),"psi_min","io",ConfigType::Float,&psiCfg_.minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> psiMaxVar_{NVS_KEY("io_psx"),"psi_max","io",ConfigType::Float,&psiCfg_.maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> waterSourceVar_{NVS_KEY("io_tws"),"water_source","io",ConfigType::UInt8,&waterCfg_.source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> waterChannelVar_{NVS_KEY("io_twc"),"water_channel","io",ConfigType::UInt8,&waterCfg_.channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> waterC0Var_{NVS_KEY("io_tw0"),"water_c0","io",ConfigType::Float,&waterCfg_.c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> waterC1Var_{NVS_KEY("io_tw1"),"water_c1","io",ConfigType::Float,&waterCfg_.c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> waterPrecVar_{NVS_KEY("io_twp"),"water_prec","io",ConfigType::Int32,&waterCfg_.precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> waterMinVar_{NVS_KEY("io_twn"),"water_min","io",ConfigType::Float,&waterCfg_.minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> waterMaxVar_{NVS_KEY("io_twx"),"water_max","io",ConfigType::Float,&waterCfg_.maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> airSourceVar_{NVS_KEY("io_tas"),"air_source","io",ConfigType::UInt8,&airCfg_.source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> airChannelVar_{NVS_KEY("io_tac"),"air_channel","io",ConfigType::UInt8,&airCfg_.channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> airC0Var_{NVS_KEY("io_ta0"),"air_c0","io",ConfigType::Float,&airCfg_.c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> airC1Var_{NVS_KEY("io_ta1"),"air_c1","io",ConfigType::Float,&airCfg_.c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> airPrecVar_{NVS_KEY("io_tap"),"air_prec","io",ConfigType::Int32,&airCfg_.precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> airMinVar_{NVS_KEY("io_tan"),"air_min","io",ConfigType::Float,&airCfg_.minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> airMaxVar_{NVS_KEY("io_tax"),"air_max","io",ConfigType::Float,&airCfg_.maxValid,ConfigPersistence::Persistent,0};
};
