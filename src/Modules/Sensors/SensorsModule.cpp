/**
 * @file SensorsModule.cpp
 * @brief Implementation file.
 */
#include "SensorsModule.h"
#define LOG_TAG "Sensors"
#include "Core/ModuleLog.h"
#include "Modules/Sensors/SensorsRuntime.h"
#include <Arduino.h>
#include <string.h>
#include <Wire.h>
#include "freertos/semphr.h"
#include <math.h>

void SensorsModule::init(ConfigStore& cfg, I2CManager&, ServiceRegistry& services) {
    logHub = services.get<LogHubService>("loghub");
    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore = dsSvc ? dsSvc->store : nullptr;

    cfg.registerVar(enabledVar);
    cfg.registerVar(oneWirePinVar);
    cfg.registerVar(oneWireAirPinVar);
    cfg.registerVar(adcModeVar);
    cfg.registerVar(pollMsVar);
    cfg.registerVar(adcPollMsVar);
    cfg.registerVar(i2cSdaVar);
    cfg.registerVar(i2cSclVar);
    cfg.registerVar(adsAddrVar);
    cfg.registerVar(adsAddr2Var);
    cfg.registerVar(adsGainVar);
    cfg.registerVar(adsRateVar);
    cfg.registerVar(adsMinVar);
    cfg.registerVar(adsMaxVar);
    cfg.registerVar(phC0Var);
    cfg.registerVar(phC1Var);
    cfg.registerVar(phPrecVar);
    cfg.registerVar(orpC0Var);
    cfg.registerVar(orpC1Var);
    cfg.registerVar(orpPrecVar);
    cfg.registerVar(psiC0Var);
    cfg.registerVar(psiC1Var);
    cfg.registerVar(psiPrecVar);
    cfg.registerVar(waterC0Var);
    cfg.registerVar(waterC1Var);
    cfg.registerVar(waterPrecVar);
    cfg.registerVar(airC0Var);
    cfg.registerVar(airC1Var);
    cfg.registerVar(airPrecVar);
    cfg.registerVar(virtualActuatorsVar);

    if (!cfgData.enabled) {
        LOGW("Sensors disabled");
        return;
    }

    i2cMutex = xSemaphoreCreateMutex();

    Wire.begin(cfgData.i2cSda, cfgData.i2cScl);

    adcRangeFilter.setRange(cfgData.adsMin, cfgData.adsMax);

    if (!adsPrimary) adsPrimary = new ADS1115(cfgData.adsAddr, &Wire);
    adsPrimaryOk = adsPrimary->begin() && adsPrimary->isConnected();
    if (!adsPrimaryOk) {
        LOGW("ADS1115 primary not found (addr=0x%02X)", cfgData.adsAddr);
    } else {
        adsPrimary->setGain((uint8_t)cfgData.adsGain);
        adsPrimary->setDataRate((uint8_t)cfgData.adsRate);
    }

    if (!useExternalPhOrpOverride) {
        useExternalPhOrp = (cfgData.adcMode != 0);
    }
    if (useExternalPhOrp) {
        if (!adsSecondary) adsSecondary = new ADS1115(cfgData.adsAddr2, &Wire);
        adsSecondaryOk = adsSecondary->begin() && adsSecondary->isConnected();
        if (!adsSecondaryOk) {
            LOGW("ADS1115 secondary not found (addr=0x%02X)", cfgData.adsAddr2);
        } else {
            adsSecondary->setGain((uint8_t)cfgData.adsGain);
            adsSecondary->setDataRate((uint8_t)cfgData.adsRate);
        }
    }

    if (!oneWireWater) {
        oneWireWater = new OneWireBus(cfgData.onewirePinWater);
        oneWireWater->begin();
    }
    if (!oneWireAir) {
        oneWireAir = new OneWireBus(cfgData.onewirePinAir);
        oneWireAir->begin();
    }

    setupDallasAddresses();
    setupSensors();

    LOGI("Sensors ready (poll=%dms, ph/orp=%s, psi=internal)", cfgData.pollMs,
         useExternalPhOrp ? "external" : "internal");

    xTaskCreatePinnedToCore(
        SensorsModule::adcTaskEntry, "sensors_adc", 4096,
        this, 1, &adcTaskHandle, 1
    );
}

void SensorsModule::setupDallasAddresses() {
    if (!oneWireWater || !oneWireAir) return;

    uint8_t countW = oneWireWater->deviceCount();
    uint8_t countA = oneWireAir->deviceCount();

    if (countW == 0) {
        LOGW("No DS18B20 devices found on water bus");
    } else if (oneWireWater->getAddress(0, waterAddr)) {
        LOGI("Water temperature sensor bound to bus water index 0");
    }

    if (countA == 0) {
        LOGW("No DS18B20 devices found on air bus");
    } else if (oneWireAir->getAddress(0, airAddr)) {
        LOGI("Air temperature sensor bound to bus air index 0");
    }
}

void SensorsModule::setupSensors() {
    waterTempDriver = new DallasTempDriver("water_temp", oneWireWater, waterAddr);
    airTempDriver = new DallasTempDriver("air_temp", oneWireAir, airAddr);

    phSensor = new SensorPipeline("ph", &phRaw, phFilters, 3);
    orpSensor = new SensorPipeline("orp", &orpRaw, orpFilters, 3);
    pumpSensor = new SensorPipeline("pump_pressure", &pumpRaw, pumpFilters, 3);
    waterSensor = new SensorPipeline("water_temp", waterTempDriver, waterFilters, 3);
    airSensor = new SensorPipeline("air_temp", airTempDriver, airFilters, 3);

    registry.add(phSensor);
    registry.add(orpSensor);
    registry.add(pumpSensor);

    if (waterTempDriver->hasAddress()) registry.add(waterSensor);
    if (airTempDriver->hasAddress()) registry.add(airSensor);

    if (cfgData.virtualActuators && dataStore) {
        for (uint8_t i = 0; i < ACTUATOR_MAX; ++i) {
            snprintf(actTankNames[i], sizeof(actTankNames[i]), "act%u_tank", i);
            snprintf(actUptimeNames[i], sizeof(actUptimeNames[i]), "act%u_uptime_h", i);
            actTankDrivers[i] = new ActuatorVirtualDriver(actTankNames[i], dataStore, i,
                                                          ActuatorVirtualDriver::Metric::TankFillPct);
            actUptimeDrivers[i] = new ActuatorVirtualDriver(actUptimeNames[i], dataStore, i,
                                                            ActuatorVirtualDriver::Metric::UptimeHours);
            actTankSensors[i] = new SensorPipeline(actTankNames[i], actTankDrivers[i], nullptr, 0);
            actUptimeSensors[i] = new SensorPipeline(actUptimeNames[i], actUptimeDrivers[i], nullptr, 0);
            registry.add(actTankSensors[i]);
            registry.add(actUptimeSensors[i]);
        }
    }

    LOGI("Sensors registered: %u", registry.count());
}

void SensorsModule::adcTaskEntry(void* arg) {
    SensorsModule* self = static_cast<SensorsModule*>(arg);
    self->adcLoop();
}

void SensorsModule::lockI2C() {
    if (!i2cMutex) return;
    xSemaphoreTake(i2cMutex, portMAX_DELAY);
}

void SensorsModule::unlockI2C() {
    if (!i2cMutex) return;
    xSemaphoreGive(i2cMutex);
}

float SensorsModule::applyPrecision(float value, int32_t decimals) const
{
    if (decimals <= 0) return (float)((int32_t)lroundf(value));
    if (decimals == 1) return roundf(value * 10.0f) / 10.0f;
    if (decimals == 2) return roundf(value * 100.0f) / 100.0f;
    float scale = 1.0f;
    for (int32_t i = 0; i < decimals; ++i) scale *= 10.0f;
    return roundf(value * scale) / scale;
}

float SensorsModule::adsToMilliVolts(ADS1115* ads, int16_t raw) const
{
    if (!ads) return (float)raw * 0.1875f;
    float v = ads->toVoltage(raw); // volts
    if (v <= ADS1X15_INVALID_VOLTAGE) {
        return (float)raw * 0.1875f;
    }
    return v * 1000.0f; // mV
}

void SensorsModule::applyAdcSample(CachedSensor& target, float value, bool valid, SensorPipeline* pipeline) {
    SensorReading raw;
    raw.timestampMs = millis();
    raw.valid = valid;
    raw.value = value;
    target.update(raw);

    if (!pipeline) return;

    uint32_t now = millis();
    bool shouldLog = (uint32_t)(now - lastAdcLogMs) >= 5000;
    if (shouldLog) lastAdcLogMs = now;

    SensorReading filtered = pipeline->read();
    if (filtered.valid) {
        if (dataStore) {
            if (pipeline == phSensor) {
                float v = applyCalibration(filtered.value, cfgData.phC0, cfgData.phC1);
                v = applyPrecision(v, cfgData.phPrec);
                setSensorsPh(*dataStore, v);
            } else if (pipeline == orpSensor) {
                float v = applyCalibration(filtered.value, cfgData.orpC0, cfgData.orpC1);
                v = applyPrecision(v, cfgData.orpPrec);
                setSensorsOrp(*dataStore, v);
            } else if (pipeline == pumpSensor) {
                float v = applyCalibration(filtered.value, cfgData.psiC0, cfgData.psiC1);
                v = applyPrecision(v, cfgData.psiPrec);
                setSensorsPsi(*dataStore, v);
            }
        }
    } else {
        if (shouldLog) LOGW("%s invalid", pipeline->name());
    }
}

void SensorsModule::adcLoop() {
    while (!cfgData.enabled) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (cfgData.adcPollMs < 20) cfgData.adcPollMs = 20;
    TickType_t period = pdMS_TO_TICKS(cfgData.adcPollMs);
    TickType_t tick = xTaskGetTickCount();

    if (adsPrimaryOk && adsPrimary) {
        lockI2C();
        if (useExternalPhOrp) {
            adsPrimary->requestADC(2); // PSI on channel 2
            adcChannelIndex = 2;
        } else {
            adsPrimary->requestADC(0); // single ADS: ch0,1,2
            adcChannelIndex = 0;
        }
        adcRequested = true;
        unlockI2C();
    }

    if (useExternalPhOrp && adsSecondaryOk && adsSecondary) {
        lockI2C();
        adsSecondary->requestADC_Differential_0_1(); // ORP diff 0-1
        adcSecondaryRequested = true;
        adcSecondaryIndex = 0;
        unlockI2C();
    }

    for (;;) {
        if (adsPrimaryOk && adsPrimary) {
            lockI2C();
            if (adcRequested && adsPrimary->isReady()) {
                int16_t v = adsPrimary->getValue();
                if (useExternalPhOrp) {
                    float mv = adsToMilliVolts(adsPrimary, v);
                    applyAdcSample(pumpRaw, mv, true, pumpSensor);
                    adsPrimary->requestADC(2);
                } else {
                    if (adcChannelIndex == 0) {
                        float mv = adsToMilliVolts(adsPrimary, v);
                        applyAdcSample(phRaw, mv, true, phSensor);
                    } else if (adcChannelIndex == 1) {
                        float mv = adsToMilliVolts(adsPrimary, v);
                        applyAdcSample(orpRaw, mv, true, orpSensor);
                    } else if (adcChannelIndex == 2) {
                        float mv = adsToMilliVolts(adsPrimary, v);
                        applyAdcSample(pumpRaw, mv, true, pumpSensor);
                    }
                    adcChannelIndex = (uint8_t)((adcChannelIndex + 1) % 3);
                    adsPrimary->requestADC(adcChannelIndex);
                }
            }
            unlockI2C();
        }

        if (useExternalPhOrp && adsSecondaryOk && adsSecondary) {
            lockI2C();
            if (adcSecondaryRequested && adsSecondary->isReady()) {
                int16_t v = adsSecondary->getValue();
                if (adcSecondaryIndex == 0) {
                    float mv = adsToMilliVolts(adsSecondary, v);
                    applyAdcSample(orpRaw, mv, true, orpSensor);
                    adcSecondaryIndex = 1;
                    adsSecondary->requestADC_Differential_2_3();
                } else {
                    float mv = adsToMilliVolts(adsSecondary, v);
                    applyAdcSample(phRaw, mv, true, phSensor);
                    adcSecondaryIndex = 0;
                    adsSecondary->requestADC_Differential_0_1();
                }
            }
            unlockI2C();
        }

        vTaskDelayUntil(&tick, period);
    }
}

void SensorsModule::loop() {
    if (!cfgData.enabled) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }

    uint32_t now = millis();
    if ((uint32_t)(now - lastPollMs) < (uint32_t)cfgData.pollMs) {
        vTaskDelay(pdMS_TO_TICKS(20));
        return;
    }
    lastPollMs = now;

    if (!tempPrimed) {
        if (oneWireWater) oneWireWater->request();
        if (oneWireAir) oneWireAir->request();
        tempPrimed = true;
        vTaskDelay(pdMS_TO_TICKS(20));
        return;
    }

    if (waterSensor) {
        SensorReading r = waterSensor->read();
        if (r.valid) {
            if (dataStore) {
                float v = applyCalibration(r.value, cfgData.waterTempC0, cfgData.waterTempC1);
                v = applyPrecision(v, cfgData.waterTempPrec);
                setSensorsWaterTemp(*dataStore, v);
            }
        } else {
            LOGW("%s invalid", waterSensor->name());
        }
    }
    if (airSensor) {
        SensorReading r = airSensor->read();
        if (r.valid) {
            if (dataStore) {
                float v = applyCalibration(r.value, cfgData.airTempC0, cfgData.airTempC1);
                v = applyPrecision(v, cfgData.airTempPrec);
                setSensorsAirTemp(*dataStore, v);
            }
        } else {
            LOGW("%s invalid", airSensor->name());
        }
    }

    if (oneWireWater) oneWireWater->request();
    if (oneWireAir) oneWireAir->request();

    vTaskDelay(pdMS_TO_TICKS(20));
}
