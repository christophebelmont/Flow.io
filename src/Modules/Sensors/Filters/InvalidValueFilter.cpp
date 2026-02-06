/**
 * @file InvalidValueFilter.cpp
 * @brief Implementation file.
 */
#include "InvalidValueFilter.h"
#include <math.h>

SensorReading InvalidValueFilter::apply(const SensorReading& in) {
    SensorReading out = in;
    if (!isfinite(out.value)) {
        out.valid = false;
        out.value = 0.0f;
    }
    return out;
}
