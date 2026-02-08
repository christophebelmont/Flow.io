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

typedef void (*IOAnalogValueCallback)(void* ctx, float value);

struct IOAnalogDefinition {
    char id[24] = {0};
    uint8_t source = IO_SRC_ADS_INTERNAL_SINGLE;
    uint8_t channel = 0;
    float c0 = 1.0f;
    float c1 = 0.0f;
    int32_t precision = 1;
    float minValid = -32768.0f;
    float maxValid = 32767.0f;
    IOAnalogValueCallback onValueChanged = nullptr;
    void* onValueCtx = nullptr;
};

struct IOAnalogSlotConfig {
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

    static constexpr uint8_t MAX_ANALOG_ENDPOINTS = 12;
    static constexpr uint8_t ANALOG_CFG_SLOTS = 5;

    struct AnalogSlot {
        bool used = false;
        IOAnalogDefinition def{};
        AnalogSensorEndpoint* endpoint = nullptr;
        RunningMedianAverageFloat median{11, 5};
        bool lastRoundedValid = false;
        float lastRounded = 0.0f;
    };

    IOModuleConfig cfgData_{};
    IOAnalogSlotConfig analogCfg_[ANALOG_CFG_SLOTS]{};

    const LogHubService* logHub_ = nullptr;
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

    ConfigVariable<bool,0> enabledVar_ { NVS_KEY("io_en"),"enabled","io",ConfigType::Bool,&cfgData_.enabled,ConfigPersistence::Persistent,0 };
    ConfigVariable<int32_t,0> i2cSdaVar_ { NVS_KEY("io_sda"),"i2c_sda","io",ConfigType::Int32,&cfgData_.i2cSda,ConfigPersistence::Persistent,0 };
    ConfigVariable<int32_t,0> i2cSclVar_ { NVS_KEY("io_scl"),"i2c_scl","io",ConfigType::Int32,&cfgData_.i2cScl,ConfigPersistence::Persistent,0 };
    ConfigVariable<int32_t,0> adsPollVar_ { NVS_KEY("io_ads"),"ads_poll_ms","io",ConfigType::Int32,&cfgData_.adsPollMs,ConfigPersistence::Persistent,0 };
    ConfigVariable<int32_t,0> dsPollVar_ { NVS_KEY("io_ds"),"ds_poll_ms","io",ConfigType::Int32,&cfgData_.dsPollMs,ConfigPersistence::Persistent,0 };
    ConfigVariable<uint8_t,0> adsInternalAddrVar_ { NVS_KEY("io_aiad"),"ads_internal_addr","io",ConfigType::UInt8,&cfgData_.adsInternalAddr,ConfigPersistence::Persistent,0 };
    ConfigVariable<uint8_t,0> adsExternalAddrVar_ { NVS_KEY("io_aead"),"ads_external_addr","io",ConfigType::UInt8,&cfgData_.adsExternalAddr,ConfigPersistence::Persistent,0 };
    ConfigVariable<int32_t,0> adsGainVar_ { NVS_KEY("io_agai"),"ads_gain","io",ConfigType::Int32,&cfgData_.adsGain,ConfigPersistence::Persistent,0 };
    ConfigVariable<int32_t,0> adsRateVar_ { NVS_KEY("io_arat"),"ads_rate","io",ConfigType::Int32,&cfgData_.adsRate,ConfigPersistence::Persistent,0 };
    ConfigVariable<bool,0> pcfEnabledVar_ { NVS_KEY("io_pcfen"),"pcf_enabled","io",ConfigType::Bool,&cfgData_.pcfEnabled,ConfigPersistence::Persistent,0 };
    ConfigVariable<uint8_t,0> pcfAddressVar_ { NVS_KEY("io_pcfad"),"pcf_address","io",ConfigType::UInt8,&cfgData_.pcfAddress,ConfigPersistence::Persistent,0 };
    ConfigVariable<uint8_t,0> pcfMaskDefaultVar_ { NVS_KEY("io_pcfmk"),"pcf_mask_default","io",ConfigType::UInt8,&cfgData_.pcfMaskDefault,ConfigPersistence::Persistent,0 };

    ConfigVariable<uint8_t,0> a0SourceVar_{NVS_KEY("io_a0s"),"a0_source","io",ConfigType::UInt8,&analogCfg_[0].source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> a0ChannelVar_{NVS_KEY("io_a0c"),"a0_channel","io",ConfigType::UInt8,&analogCfg_[0].channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a0C0Var_{NVS_KEY("io_a00"),"a0_c0","io",ConfigType::Float,&analogCfg_[0].c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a0C1Var_{NVS_KEY("io_a01"),"a0_c1","io",ConfigType::Float,&analogCfg_[0].c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> a0PrecVar_{NVS_KEY("io_a0p"),"a0_prec","io",ConfigType::Int32,&analogCfg_[0].precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a0MinVar_{NVS_KEY("io_a0n"),"a0_min","io",ConfigType::Float,&analogCfg_[0].minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a0MaxVar_{NVS_KEY("io_a0x"),"a0_max","io",ConfigType::Float,&analogCfg_[0].maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> a1SourceVar_{NVS_KEY("io_a1s"),"a1_source","io",ConfigType::UInt8,&analogCfg_[1].source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> a1ChannelVar_{NVS_KEY("io_a1c"),"a1_channel","io",ConfigType::UInt8,&analogCfg_[1].channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a1C0Var_{NVS_KEY("io_a10"),"a1_c0","io",ConfigType::Float,&analogCfg_[1].c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a1C1Var_{NVS_KEY("io_a11"),"a1_c1","io",ConfigType::Float,&analogCfg_[1].c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> a1PrecVar_{NVS_KEY("io_a1p"),"a1_prec","io",ConfigType::Int32,&analogCfg_[1].precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a1MinVar_{NVS_KEY("io_a1n"),"a1_min","io",ConfigType::Float,&analogCfg_[1].minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a1MaxVar_{NVS_KEY("io_a1x"),"a1_max","io",ConfigType::Float,&analogCfg_[1].maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> a2SourceVar_{NVS_KEY("io_a2s"),"a2_source","io",ConfigType::UInt8,&analogCfg_[2].source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> a2ChannelVar_{NVS_KEY("io_a2c"),"a2_channel","io",ConfigType::UInt8,&analogCfg_[2].channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a2C0Var_{NVS_KEY("io_a20"),"a2_c0","io",ConfigType::Float,&analogCfg_[2].c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a2C1Var_{NVS_KEY("io_a21"),"a2_c1","io",ConfigType::Float,&analogCfg_[2].c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> a2PrecVar_{NVS_KEY("io_a2p"),"a2_prec","io",ConfigType::Int32,&analogCfg_[2].precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a2MinVar_{NVS_KEY("io_a2n"),"a2_min","io",ConfigType::Float,&analogCfg_[2].minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a2MaxVar_{NVS_KEY("io_a2x"),"a2_max","io",ConfigType::Float,&analogCfg_[2].maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> a3SourceVar_{NVS_KEY("io_a3s"),"a3_source","io",ConfigType::UInt8,&analogCfg_[3].source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> a3ChannelVar_{NVS_KEY("io_a3c"),"a3_channel","io",ConfigType::UInt8,&analogCfg_[3].channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a3C0Var_{NVS_KEY("io_a30"),"a3_c0","io",ConfigType::Float,&analogCfg_[3].c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a3C1Var_{NVS_KEY("io_a31"),"a3_c1","io",ConfigType::Float,&analogCfg_[3].c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> a3PrecVar_{NVS_KEY("io_a3p"),"a3_prec","io",ConfigType::Int32,&analogCfg_[3].precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a3MinVar_{NVS_KEY("io_a3n"),"a3_min","io",ConfigType::Float,&analogCfg_[3].minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a3MaxVar_{NVS_KEY("io_a3x"),"a3_max","io",ConfigType::Float,&analogCfg_[3].maxValid,ConfigPersistence::Persistent,0};

    ConfigVariable<uint8_t,0> a4SourceVar_{NVS_KEY("io_a4s"),"a4_source","io",ConfigType::UInt8,&analogCfg_[4].source,ConfigPersistence::Persistent,0};
    ConfigVariable<uint8_t,0> a4ChannelVar_{NVS_KEY("io_a4c"),"a4_channel","io",ConfigType::UInt8,&analogCfg_[4].channel,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a4C0Var_{NVS_KEY("io_a40"),"a4_c0","io",ConfigType::Float,&analogCfg_[4].c0,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a4C1Var_{NVS_KEY("io_a41"),"a4_c1","io",ConfigType::Float,&analogCfg_[4].c1,ConfigPersistence::Persistent,0};
    ConfigVariable<int32_t,0> a4PrecVar_{NVS_KEY("io_a4p"),"a4_prec","io",ConfigType::Int32,&analogCfg_[4].precision,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a4MinVar_{NVS_KEY("io_a4n"),"a4_min","io",ConfigType::Float,&analogCfg_[4].minValid,ConfigPersistence::Persistent,0};
    ConfigVariable<float,0> a4MaxVar_{NVS_KEY("io_a4x"),"a4_max","io",ConfigType::Float,&analogCfg_[4].maxValid,ConfigPersistence::Persistent,0};
};
