#pragma once
/**
 * @file RunningMedianFilter.h
 * @brief Running median filter.
 */
#include "Modules/Sensors/Engine/SensorPipeline.h"

class RunningMedianFilter : public ISensorFilter {
public:
    explicit RunningMedianFilter(size_t windowSize);
    ~RunningMedianFilter();

    SensorReading apply(const SensorReading& in) override;

private:
    size_t window_ = 0;
    float* buffer_ = nullptr;
    float* scratch_ = nullptr;
    size_t index_ = 0;
    size_t count_ = 0;
};
