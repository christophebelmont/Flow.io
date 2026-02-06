#pragma once
/**
 * @file ISensor.h
 * @brief Base interface for all sensors.
 */
#include "SensorReading.h"

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual const char* name() const = 0;
    virtual SensorReading read() = 0;
};
