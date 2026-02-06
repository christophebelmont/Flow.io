#pragma once
/**
 * @file DallasTempDriver.h
 * @brief DS18B20 temperature driver.
 */
#include "Modules/Sensors/Engine/ISensor.h"
#include "Modules/Sensors/Bus/OneWireBus.h"

class DallasTempDriver : public ISensor {
public:
    DallasTempDriver(const char* sensorName, OneWireBus* bus, const uint8_t addr[8]);

    const char* name() const override { return name_; }
    SensorReading read() override;
    bool hasAddress() const { return hasAddress_; }

private:
    const char* name_ = nullptr;
    OneWireBus* bus_ = nullptr;
    uint8_t addr_[8] = {0};
    bool hasAddress_ = false;
};
