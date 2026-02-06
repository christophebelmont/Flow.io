#pragma once
/**
 * @file SensorRegistry.h
 * @brief Fixed-size registry for sensors.
 */
#include "ISensor.h"

constexpr size_t MAX_SENSORS = 24;

class SensorRegistry {
public:
    bool add(ISensor* s) {
        if (!s || count_ >= MAX_SENSORS) return false;
        sensors_[count_++] = s;
        return true;
    }

    uint8_t count() const { return count_; }
    ISensor* at(uint8_t idx) const {
        if (idx >= count_) return nullptr;
        return sensors_[idx];
    }

private:
    ISensor* sensors_[MAX_SENSORS] = {nullptr};
    uint8_t count_ = 0;
};
