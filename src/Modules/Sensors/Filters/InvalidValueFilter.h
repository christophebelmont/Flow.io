#pragma once
/**
 * @file InvalidValueFilter.h
 * @brief Invalid value handling.
 */
#include "Modules/Sensors/Engine/SensorPipeline.h"

class InvalidValueFilter : public ISensorFilter {
public:
    SensorReading apply(const SensorReading& in) override;
};
