/**
 * @file Ads1115Driver.cpp
 * @brief Implementation file.
 */
#include "Ads1115Driver.h"
#include <Arduino.h>

Ads1115Driver::Ads1115Driver(const char* sensorName, ADS1115* ads, uint8_t channel)
    : name_(sensorName), ads_(ads), channel_(channel) {}

SensorReading Ads1115Driver::read() {
    SensorReading r;
    r.timestampMs = millis();

    if (!ads_) {
        r.valid = false;
        return r;
    }

    int16_t v = ads_->readADC(channel_);
    r.value = (float)v;
    r.valid = true;
    return r;
}
