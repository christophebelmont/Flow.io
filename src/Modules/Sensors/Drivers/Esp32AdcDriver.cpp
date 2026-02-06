/**
 * @file Esp32AdcDriver.cpp
 * @brief Implementation file.
 */
#include "Esp32AdcDriver.h"
#include <Arduino.h>

Esp32AdcDriver::Esp32AdcDriver(const char* sensorName, AdcBus* bus, int pin)
    : name_(sensorName), bus_(bus), pin_(pin) {}

SensorReading Esp32AdcDriver::read() {
    SensorReading r;
    r.timestampMs = millis();

    if (!bus_ || pin_ < 0) {
        r.valid = false;
        return r;
    }

    int v = bus_->read(pin_);
    r.value = (float)v;
    r.valid = true;
    return r;
}
