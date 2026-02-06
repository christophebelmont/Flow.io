#pragma once
/**
 * @file RunningMedianAverageFilter.h
 * @brief RunningMedian filter using RobTillaart/RunningMedian.
 */
#include "Modules/Sensors/Engine/SensorPipeline.h"
#include <RunningMedian.h>

class RunningMedianAverageFilter : public ISensorFilter {
public:
    RunningMedianAverageFilter(uint8_t windowSize, uint8_t avgCount);

    SensorReading apply(const SensorReading& in) override;

private:
    RunningMedian rm_;
    uint8_t avgCount_ = 0;
};
