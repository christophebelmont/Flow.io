/**
 * @file OneWireBus.cpp
 * @brief Implementation file.
 */

#include "OneWireBus.h"
#include <OneWire.h>
#include <DallasTemperature.h>

OneWireBus::OneWireBus(int pin) : pin_(pin) {}

OneWireBus::~OneWireBus() {
    if (dt_) delete dt_;
    if (oneWire_) delete oneWire_;
}

void OneWireBus::begin() {
    if (pin_ < 0) return;
    if (oneWire_ || dt_) return;

    oneWire_ = new OneWire(pin_);
    dt_ = new DallasTemperature(oneWire_);
    dt_->begin();
    dt_->setWaitForConversion(false);
}

void OneWireBus::request() {
    if (!dt_) return;
    dt_->requestTemperatures();
}

void OneWireBus::setWaitForConversion(bool enabled) {
    if (!dt_) return;
    dt_->setWaitForConversion(enabled);
}

bool OneWireBus::getAddress(uint8_t index, uint8_t out[8]) const {
    if (!dt_ || !out) return false;
    return dt_->getAddress(out, index);
}

float OneWireBus::readC(const uint8_t addr[8]) const {
    if (!dt_ || !addr) return DEVICE_DISCONNECTED_C;
    return dt_->getTempC(addr);
}

uint8_t OneWireBus::deviceCount() const {
    if (!dt_) return 0;
    return (uint8_t)dt_->getDeviceCount();
}
