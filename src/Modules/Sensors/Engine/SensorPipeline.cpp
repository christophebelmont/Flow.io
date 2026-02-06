/**
 * @file SensorPipeline.cpp
 * @brief Implementation file.
 */
#include "SensorPipeline.h"
#include <cstddef>

SensorPipeline::SensorPipeline(const char* sensorName, ISensor* source, ISensorFilter** filters, size_t filterCount)
    : sensorName_(sensorName)
    , source_(source)
    , filters_(filters)
    , filterCount_(filterCount)
{}

SensorReading SensorPipeline::read() {
    if (!source_) return SensorReading{};
    SensorReading out = source_->read();

    if (!filters_ || filterCount_ == 0) return out;

    for (size_t i = 0; i < filterCount_; ++i) {
        if (!filters_[i]) continue;
        out = filters_[i]->apply(out);
    }
    return out;
}
