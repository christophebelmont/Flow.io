#pragma once
/**
 * @file Ads1115Driver.h
 * @brief ADS1115 driver (RobTillaart/ADS1X15).
 */
#include "Modules/Sensors/Engine/ISensor.h"
#include <ADS1X15.h>

class Ads1115Driver : public ISensor {
public:
    Ads1115Driver(const char* sensorName, ADS1115* ads, uint8_t channel);

    const char* name() const override { return name_; }
    SensorReading read() override;

private:
    const char* name_ = nullptr;
    ADS1115* ads_ = nullptr;
    uint8_t channel_ = 0;
};
