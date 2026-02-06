/**
 * @file RunningMedianAverageFilter.cpp
 * @brief Implementation file.
 */
#include "RunningMedianAverageFilter.h"

RunningMedianAverageFilter::RunningMedianAverageFilter(uint8_t windowSize, uint8_t avgCount)
    : rm_(windowSize), avgCount_(avgCount) {}

SensorReading RunningMedianAverageFilter::apply(const SensorReading& in) {
    if (!in.valid) return in;

    rm_.add(in.value);

    SensorReading out = in;
    uint8_t count = rm_.getCount();
    if (count == 0) return out;

    uint8_t n = avgCount_;
    if (n == 0) return out;
    if (n > count) n = count;

    out.value = rm_.getAverage(n);
    return out;
}
