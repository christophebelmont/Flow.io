#pragma once
/**
 * @file RangeFilter.h
 * @brief Range validation filter.
 */
#include "Modules/Sensors/Engine/SensorPipeline.h"

class RangeFilter : public ISensorFilter {
public:
    RangeFilter(float minValue, float maxValue);
    void setRange(float minValue, float maxValue);
    SensorReading apply(const SensorReading& in) override;

private:
    float min_ = 0.0f;
    float max_ = 0.0f;
};
