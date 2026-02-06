#pragma once
/**
 * @file SensorReading.h
 * @brief Common sensor reading struct.
 */
#include <stdint.h>

struct SensorReading {
    float value = 0.0f;
    bool valid = false;
    uint32_t timestampMs = 0;
};
