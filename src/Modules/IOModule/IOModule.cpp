/**
 * @file IOModule.cpp
 * @brief Implementation file.
 */

#include "IOModule.h"
#define LOG_TAG "IOModule"
#include "Core/ModuleLog.h"
#include "Modules/IOModule/IORuntime.h"
#include <Arduino.h>

void IOModule::setOneWireBuses(OneWireBus* water, OneWireBus* air)
{
    oneWireWater_ = water;
    oneWireAir_ = air;
}

bool IOModule::defineAnalogInput(const IOAnalogDefinition& def)
{
    if (def.id[0] == '\0') return false;

    for (uint8_t i = 0; i < MAX_ANALOG_ENDPOINTS; ++i) {
        if (analogSlots_[i].used) continue;
        analogSlots_[i].used = true;
        analogSlots_[i].def = def;

        if (i < ANALOG_CFG_SLOTS) {
            analogCfg_[i].source = def.source;
            analogCfg_[i].channel = def.channel;
            analogCfg_[i].c0 = def.c0;
            analogCfg_[i].c1 = def.c1;
            analogCfg_[i].precision = def.precision;
            analogCfg_[i].minValid = def.minValid;
            analogCfg_[i].maxValid = def.maxValid;
        }

        return true;
    }

    return false;
}

bool IOModule::tickFastAds_(void* ctx, uint32_t nowMs)
{
    IOModule* self = static_cast<IOModule*>(ctx);
    if (!self || !self->runtimeReady_) return false;

    if (self->adsInternal_) self->adsInternal_->tick(nowMs);
    if (self->adsExternal_) self->adsExternal_->tick(nowMs);

    for (uint8_t i = 0; i < MAX_ANALOG_ENDPOINTS; ++i) {
        if (!self->analogSlots_[i].used) continue;
        uint8_t src = self->analogSlots_[i].def.source;
        if (src == IO_SRC_ADS_INTERNAL_SINGLE || src == IO_SRC_ADS_EXTERNAL_DIFF) {
            self->processAnalogDefinition_(i, nowMs);
        }
    }
    return true;
}

bool IOModule::tickSlowDs_(void* ctx, uint32_t nowMs)
{
    IOModule* self = static_cast<IOModule*>(ctx);
    if (!self || !self->runtimeReady_) return false;

    if (self->dsWater_) self->dsWater_->tick(nowMs);
    if (self->dsAir_) self->dsAir_->tick(nowMs);

    for (uint8_t i = 0; i < MAX_ANALOG_ENDPOINTS; ++i) {
        if (!self->analogSlots_[i].used) continue;
        uint8_t src = self->analogSlots_[i].def.source;
        if (src == IO_SRC_DS18_WATER || src == IO_SRC_DS18_AIR) {
            self->processAnalogDefinition_(i, nowMs);
        }
    }
    return true;
}

bool IOModule::processAnalogDefinition_(uint8_t idx, uint32_t nowMs)
{
    if (idx >= MAX_ANALOG_ENDPOINTS) return false;
    AnalogSlot& slot = analogSlots_[idx];
    if (!slot.used || !slot.endpoint) return false;

    float raw = 0.0f;
    bool valid = false;

    if (slot.def.source == IO_SRC_ADS_INTERNAL_SINGLE) {
        if (!adsInternal_) return false;
        valid = adsInternal_->readMilliVoltsChannel(slot.def.channel, raw);
    } else if (slot.def.source == IO_SRC_ADS_EXTERNAL_DIFF) {
        if (!adsExternal_) return false;
        if (slot.def.channel == 0) valid = adsExternal_->readMilliVoltsDifferential01(raw);
        else valid = adsExternal_->readMilliVoltsDifferential23(raw);
    } else if (slot.def.source == IO_SRC_DS18_WATER) {
        if (!dsWater_) return false;
        valid = dsWater_->readCelsius(raw);
    } else if (slot.def.source == IO_SRC_DS18_AIR) {
        if (!dsAir_) return false;
        valid = dsAir_->readCelsius(raw);
    }

    if (!valid) return false;

    if (raw < slot.def.minValid || raw > slot.def.maxValid) {
        slot.endpoint->update(raw, false, nowMs);
        return false;
    }

    float filtered = slot.median.update(raw);
    float calibrated = (slot.def.c0 * filtered) + slot.def.c1;
    float rounded = ioRoundToPrecision(calibrated, slot.def.precision);

    slot.endpoint->update(rounded, true, nowMs);

    if (!slot.lastRoundedValid || rounded != slot.lastRounded) {
        slot.lastRounded = rounded;
        slot.lastRoundedValid = true;
        if (slot.def.onValueChanged) {
            slot.def.onValueChanged(slot.def.onValueCtx, rounded);
        }
    }

    return true;
}

bool IOModule::svcSetMask_(void* ctx, uint8_t mask)
{
    IOModule* self = static_cast<IOModule*>(ctx);
    return self ? self->setLedMask_(mask, millis()) : false;
}

bool IOModule::svcTurnOn_(void* ctx, uint8_t bit)
{
    IOModule* self = static_cast<IOModule*>(ctx);
    return self ? self->turnLedOn_(bit, millis()) : false;
}

bool IOModule::svcTurnOff_(void* ctx, uint8_t bit)
{
    IOModule* self = static_cast<IOModule*>(ctx);
    return self ? self->turnLedOff_(bit, millis()) : false;
}

bool IOModule::svcGetMask_(void* ctx, uint8_t* mask)
{
    IOModule* self = static_cast<IOModule*>(ctx);
    if (!self || !mask) return false;
    return self->getLedMask_(*mask);
}

bool IOModule::setLedMask_(uint8_t mask, uint32_t tsMs)
{
    if (!ledMaskEp_) return false;
    return ledMaskEp_->setMask(mask, tsMs);
}

bool IOModule::turnLedOn_(uint8_t bit, uint32_t tsMs)
{
    if (!ledMaskEp_) return false;
    return ledMaskEp_->turnOn(bit, tsMs);
}

bool IOModule::turnLedOff_(uint8_t bit, uint32_t tsMs)
{
    if (!ledMaskEp_) return false;
    return ledMaskEp_->turnOff(bit, tsMs);
}

bool IOModule::getLedMask_(uint8_t& mask) const
{
    if (!ledMaskEp_) return false;
    return ledMaskEp_->getMask(mask);
}

bool IOModule::configureRuntime_()
{
    if (runtimeReady_) return true;
    if (!cfgData_.enabled) return false;

    i2cBus_.begin(cfgData_.i2cSda, cfgData_.i2cScl);

    bool needAdsInternal = false;
    bool needAdsExternal = false;
    bool needDsWater = false;
    bool needDsAir = false;

    for (uint8_t i = 0; i < MAX_ANALOG_ENDPOINTS; ++i) {
        if (!analogSlots_[i].used) continue;

        if (i < ANALOG_CFG_SLOTS) {
            analogSlots_[i].def.source = analogCfg_[i].source;
            analogSlots_[i].def.channel = analogCfg_[i].channel;
            analogSlots_[i].def.c0 = analogCfg_[i].c0;
            analogSlots_[i].def.c1 = analogCfg_[i].c1;
            analogSlots_[i].def.precision = analogCfg_[i].precision;
            analogSlots_[i].def.minValid = analogCfg_[i].minValid;
            analogSlots_[i].def.maxValid = analogCfg_[i].maxValid;
        }

        if (analogSlots_[i].def.source == IO_SRC_ADS_INTERNAL_SINGLE) needAdsInternal = true;
        else if (analogSlots_[i].def.source == IO_SRC_ADS_EXTERNAL_DIFF) needAdsExternal = true;
        else if (analogSlots_[i].def.source == IO_SRC_DS18_WATER) needDsWater = true;
        else if (analogSlots_[i].def.source == IO_SRC_DS18_AIR) needDsAir = true;

        analogSlots_[i].endpoint = new AnalogSensorEndpoint(analogSlots_[i].def.id);
        registry_.add(analogSlots_[i].endpoint);
    }

    Ads1115DriverConfig adsInternalCfg{};
    adsInternalCfg.address = cfgData_.adsInternalAddr;
    adsInternalCfg.gain = (uint8_t)cfgData_.adsGain;
    adsInternalCfg.dataRate = (uint8_t)cfgData_.adsRate;
    adsInternalCfg.pollMs = (cfgData_.adsPollMs < 20) ? 20 : (uint32_t)cfgData_.adsPollMs;
    adsInternalCfg.differentialPairs = false;

    Ads1115DriverConfig adsExternalCfg = adsInternalCfg;
    adsExternalCfg.address = cfgData_.adsExternalAddr;
    adsExternalCfg.differentialPairs = true;

    if (needAdsInternal) {
        adsInternal_ = new Ads1115Driver("ads_internal", &i2cBus_, adsInternalCfg);
        if (!adsInternal_->begin()) {
            LOGW("ADS internal not detected at 0x%02X", cfgData_.adsInternalAddr);
            delete adsInternal_;
            adsInternal_ = nullptr;
        }
    }

    if (needAdsExternal) {
        adsExternal_ = new Ads1115Driver("ads_external", &i2cBus_, adsExternalCfg);
        if (!adsExternal_->begin()) {
            LOGW("ADS external not detected at 0x%02X", cfgData_.adsExternalAddr);
            delete adsExternal_;
            adsExternal_ = nullptr;
        }
    }

    Ds18b20DriverConfig dsCfg{};
    dsCfg.pollMs = (cfgData_.dsPollMs < 750) ? 750 : (uint32_t)cfgData_.dsPollMs;
    dsCfg.conversionWaitMs = 750;

    if (needDsWater && oneWireWater_) {
        oneWireWater_->begin();
        if (oneWireWater_->getAddress(0, oneWireWaterAddr_)) {
            dsWater_ = new Ds18b20Driver("ds18_water", oneWireWater_, oneWireWaterAddr_, dsCfg);
            dsWater_->begin();
        } else {
            LOGW("No DS18B20 found on water OneWire bus");
        }
    }

    if (needDsAir && oneWireAir_) {
        oneWireAir_->begin();
        if (oneWireAir_->getAddress(0, oneWireAirAddr_)) {
            dsAir_ = new Ds18b20Driver("ds18_air", oneWireAir_, oneWireAirAddr_, dsCfg);
            dsAir_->begin();
        } else {
            LOGW("No DS18B20 found on air OneWire bus");
        }
    }

    if (cfgData_.pcfEnabled) {
        pcf_ = new Pcf8574Driver("pcf8574_led", &i2cBus_, cfgData_.pcfAddress);
        if (pcf_->begin()) {
            ledMaskEp_ = new Pcf8574MaskEndpoint(
                "status_leds_mask",
                [](void* ctx, uint8_t mask) -> bool {
                    return static_cast<Pcf8574Driver*>(ctx)->writeMask(mask);
                },
                [](void* ctx, uint8_t* mask) -> bool {
                    if (!mask) return false;
                    return static_cast<Pcf8574Driver*>(ctx)->readMask(*mask);
                },
                pcf_
            );
            registry_.add(ledMaskEp_);
            ledMaskEp_->setMask(cfgData_.pcfMaskDefault, millis());
            if (services_) services_->add("io_leds", &ledSvc_);
        } else {
            LOGW("PCF8574 not detected at 0x%02X", cfgData_.pcfAddress);
            delete pcf_;
            pcf_ = nullptr;
        }
    }

    IOScheduledJob adsJob{};
    adsJob.id = "ads_fast";
    adsJob.periodMs = (cfgData_.adsPollMs < 20) ? 20 : (uint32_t)cfgData_.adsPollMs;
    adsJob.fn = &IOModule::tickFastAds_;
    adsJob.ctx = this;
    scheduler_.add(adsJob);

    IOScheduledJob dsJob{};
    dsJob.id = "ds_slow";
    dsJob.periodMs = (cfgData_.dsPollMs < 250) ? 250 : (uint32_t)cfgData_.dsPollMs;
    dsJob.fn = &IOModule::tickSlowDs_;
    dsJob.ctx = this;
    scheduler_.add(dsJob);

    runtimeReady_ = true;

    LOGI("I/O ready (ads=%ldms ds=%ldms endpoints=%u pcf=%s)",
         (long)adsJob.periodMs,
         (long)dsJob.periodMs,
         (unsigned)registry_.count(),
         cfgData_.pcfEnabled ? "on" : "off");

    return true;
}

void IOModule::init(ConfigStore& cfg, ServiceRegistry& services)
{
    services_ = &services;
    logHub_ = services.get<LogHubService>("loghub");

    cfg.registerVar(enabledVar_);
    cfg.registerVar(i2cSdaVar_);
    cfg.registerVar(i2cSclVar_);
    cfg.registerVar(adsPollVar_);
    cfg.registerVar(dsPollVar_);
    cfg.registerVar(adsInternalAddrVar_);
    cfg.registerVar(adsExternalAddrVar_);
    cfg.registerVar(adsGainVar_);
    cfg.registerVar(adsRateVar_);
    cfg.registerVar(pcfEnabledVar_);
    cfg.registerVar(pcfAddressVar_);
    cfg.registerVar(pcfMaskDefaultVar_);

    cfg.registerVar(a0SourceVar_); cfg.registerVar(a0ChannelVar_); cfg.registerVar(a0C0Var_);
    cfg.registerVar(a0C1Var_); cfg.registerVar(a0PrecVar_); cfg.registerVar(a0MinVar_); cfg.registerVar(a0MaxVar_);

    cfg.registerVar(a1SourceVar_); cfg.registerVar(a1ChannelVar_); cfg.registerVar(a1C0Var_);
    cfg.registerVar(a1C1Var_); cfg.registerVar(a1PrecVar_); cfg.registerVar(a1MinVar_); cfg.registerVar(a1MaxVar_);

    cfg.registerVar(a2SourceVar_); cfg.registerVar(a2ChannelVar_); cfg.registerVar(a2C0Var_);
    cfg.registerVar(a2C1Var_); cfg.registerVar(a2PrecVar_); cfg.registerVar(a2MinVar_); cfg.registerVar(a2MaxVar_);

    cfg.registerVar(a3SourceVar_); cfg.registerVar(a3ChannelVar_); cfg.registerVar(a3C0Var_);
    cfg.registerVar(a3C1Var_); cfg.registerVar(a3PrecVar_); cfg.registerVar(a3MinVar_); cfg.registerVar(a3MaxVar_);

    cfg.registerVar(a4SourceVar_); cfg.registerVar(a4ChannelVar_); cfg.registerVar(a4C0Var_);
    cfg.registerVar(a4C1Var_); cfg.registerVar(a4PrecVar_); cfg.registerVar(a4MinVar_); cfg.registerVar(a4MaxVar_);

    LOGI("I/O config registered");
    (void)logHub_;
}

void IOModule::loop()
{
    if (!cfgData_.enabled) {
        vTaskDelay(pdMS_TO_TICKS(500));
        return;
    }

    if (!runtimeReady_) {
        if (!configureRuntime_()) {
            vTaskDelay(pdMS_TO_TICKS(500));
            return;
        }
    }

    scheduler_.tick(millis());
    vTaskDelay(pdMS_TO_TICKS(10));
}
