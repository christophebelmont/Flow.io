/**
 * @file RangeFilter.cpp
 * @brief Implementation file.
 */
#include "RangeFilter.h"

RangeFilter::RangeFilter(float minValue, float maxValue) : min_(minValue), max_(maxValue) {}

void RangeFilter::setRange(float minValue, float maxValue) {
    min_ = minValue;
    max_ = maxValue;
}

SensorReading RangeFilter::apply(const SensorReading& in) {
    SensorReading out = in;
    if (!out.valid) return out;
    if (out.value < min_ || out.value > max_) {
        out.valid = false;
    }
    return out;
}
