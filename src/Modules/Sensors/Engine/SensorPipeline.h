#pragma once
/**
 * @file SensorPipeline.h
 * @brief Sensor wrapper that applies a chain of filters.
 */
#include "ISensor.h"
#include <cstddef>

class ISensorFilter {
public:
    virtual ~ISensorFilter() = default;
    virtual SensorReading apply(const SensorReading& in) = 0;
};

class SensorPipeline : public ISensor {
public:
    SensorPipeline(const char* sensorName, ISensor* source, ISensorFilter** filters, size_t filterCount);

    const char* name() const override { return sensorName_; }
    SensorReading read() override;

private:
    const char* sensorName_ = nullptr;
    ISensor* source_ = nullptr;
    ISensorFilter** filters_ = nullptr;
    size_t filterCount_ = 0;
};
