#pragma once
/**
 * @file Esp32AdcDriver.h
 * @brief ESP32 internal ADC driver.
 */
#include "Modules/Sensors/Engine/ISensor.h"
#include "Modules/Sensors/Bus/AdcBus.h"

class Esp32AdcDriver : public ISensor {
public:
    Esp32AdcDriver(const char* sensorName, AdcBus* bus, int pin);

    const char* name() const override { return name_; }
    SensorReading read() override;

private:
    const char* name_ = nullptr;
    AdcBus* bus_ = nullptr;
    int pin_ = -1;
};
