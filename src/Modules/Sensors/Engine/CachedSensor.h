#pragma once
/**
 * @file CachedSensor.h
 * @brief ISensor that returns the latest cached reading.
 */
#include "ISensor.h"

class CachedSensor : public ISensor {
public:
    explicit CachedSensor(const char* sensorName) : name_(sensorName) {}

    const char* name() const override { return name_; }
    SensorReading read() override { return reading_; }

    void update(const SensorReading& r) { reading_ = r; }

private:
    const char* name_ = nullptr;
    SensorReading reading_{};
};
