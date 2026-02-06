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

void SensorsModule::init(ConfigStore& cfg, I2CManager&, ServiceRegistry& services) {
    logHub = services.get<LogHubService>("loghub");
    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore = dsSvc ? dsSvc->store : nullptr;

    cfg.registerVar(enabledVar);
    cfg.registerVar(oneWirePinVar);
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

    if (!cfgData.enabled) {
        LOGW("Sensors disabled");
        return;
    }

    i2cMutex = xSemaphoreCreateMutex();

    Wire.begin(cfgData.i2cSda, cfgData.i2cScl);

    adcRangeFilter.setRange(cfgData.adsMin, cfgData.adsMax);

    adsPrimary = new ADS1115(cfgData.adsAddr, &Wire);
    adsPrimaryOk = adsPrimary->begin() && adsPrimary->isConnected();
    if (!adsPrimaryOk) {
        LOGW("ADS1115 primary not found (addr=0x%02X)", cfgData.adsAddr);
    } else {
        adsPrimary->setGain((uint8_t)cfgData.adsGain);
        adsPrimary->setDataRate((uint8_t)cfgData.adsRate);
    }

    useExternalPhOrp = (cfgData.adcMode != 0);
    if (useExternalPhOrp) {
        adsSecondary = new ADS1115(cfgData.adsAddr2, &Wire);
        adsSecondaryOk = adsSecondary->begin() && adsSecondary->isConnected();
        if (!adsSecondaryOk) {
            LOGW("ADS1115 secondary not found (addr=0x%02X)", cfgData.adsAddr2);
        } else {
            adsSecondary->setGain((uint8_t)cfgData.adsGain);
            adsSecondary->setDataRate((uint8_t)cfgData.adsRate);
        }
    }

    oneWireBus = new OneWireBus(cfgData.onewirePin);
    oneWireBus->begin();

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
    if (!oneWireBus) return;

    uint8_t count = oneWireBus->deviceCount();
    if (count == 0) {
        LOGW("No DS18B20 devices found");
        return;
    }

    if (count >= 1 && oneWireBus->getAddress(0, waterAddr)) {
        LOGI("Water temperature sensor bound to index 0");
    }
    if (count >= 2 && oneWireBus->getAddress(1, airAddr)) {
        LOGI("Air temperature sensor bound to index 1");
    }

    if (count == 1) {
        LOGW("Only one DS18B20 found; air temperature disabled");
    }
}

void SensorsModule::setupSensors() {
    waterTempDriver = new DallasTempDriver("water_temp", oneWireBus, waterAddr);
    airTempDriver = new DallasTempDriver("air_temp", oneWireBus, airAddr);

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
                setSensorsPh(*dataStore, filtered.value);
            } else if (pipeline == orpSensor) {
                setSensorsOrp(*dataStore, filtered.value);
            } else if (pipeline == pumpSensor) {
                setSensorsPsi(*dataStore, filtered.value);
            }
        }
        if (shouldLog) LOGD("%s=%.2f", pipeline->name(), filtered.value);
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
                    applyAdcSample(pumpRaw, (float)v, true, pumpSensor);
                    adsPrimary->requestADC(2);
                } else {
                    if (adcChannelIndex == 0) {
                        applyAdcSample(phRaw, (float)v, true, phSensor);
                    } else if (adcChannelIndex == 1) {
                        applyAdcSample(orpRaw, (float)v, true, orpSensor);
                    } else if (adcChannelIndex == 2) {
                        applyAdcSample(pumpRaw, (float)v, true, pumpSensor);
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
                    applyAdcSample(orpRaw, (float)v, true, orpSensor);
                    adcSecondaryIndex = 1;
                    adsSecondary->requestADC_Differential_2_3();
                } else {
                    applyAdcSample(phRaw, (float)v, true, phSensor);
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
        if (oneWireBus) oneWireBus->request();
        tempPrimed = true;
        vTaskDelay(pdMS_TO_TICKS(20));
        return;
    }

    if (waterSensor) {
        SensorReading r = waterSensor->read();
        if (r.valid) {
            if (dataStore) setSensorsWaterTemp(*dataStore, r.value);
            LOGD("%s=%.2f", waterSensor->name(), r.value);
        }
        else LOGW("%s invalid", waterSensor->name());
    }
    if (airSensor) {
        SensorReading r = airSensor->read();
        if (r.valid) {
            if (dataStore) setSensorsAirTemp(*dataStore, r.value);
            LOGD("%s=%.2f", airSensor->name(), r.value);
        }
        else LOGW("%s invalid", airSensor->name());
    }

    if (oneWireBus) {
        oneWireBus->request();
    }

    vTaskDelay(pdMS_TO_TICKS(20));
}
