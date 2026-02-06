/**
 * @file DallasTempDriver.cpp
 * @brief Implementation file.
 */
#include "DallasTempDriver.h"
#include <Arduino.h>
#include <string.h>
#include <DallasTemperature.h>

DallasTempDriver::DallasTempDriver(const char* sensorName, OneWireBus* bus, const uint8_t addr[8])
    : name_(sensorName), bus_(bus) {
    if (addr) {
        memcpy(addr_, addr, 8);
        for (int i = 0; i < 8; ++i) {
            if (addr_[i] != 0x00) {
                hasAddress_ = true;
                break;
            }
        }
    }
}

SensorReading DallasTempDriver::read() {
    SensorReading r;
    r.timestampMs = millis();

    if (!bus_ || !hasAddress_) {
        r.valid = false;
        return r;
    }

    float c = bus_->readC(addr_);
    if (c == DEVICE_DISCONNECTED_C) {
        r.valid = false;
        r.value = 0.0f;
    } else {
        r.value = c;
        r.valid = true;
    }
    return r;
}
