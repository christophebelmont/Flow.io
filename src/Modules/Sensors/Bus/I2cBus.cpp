/**
 * @file I2cBus.cpp
 * @brief Implementation file.
 */
#include "I2cBus.h"
#include <cstddef>
#include <Wire.h>

I2cBus::I2cBus(TwoWire* wire) : wire_(wire) {}

bool I2cBus::begin(int sda, int scl, uint32_t freqHz) {
    if (!wire_) return false;
    wire_->begin(sda, scl, freqHz);
    return true;
}

bool I2cBus::probe(uint8_t addr) {
    if (!wire_) return false;
    wire_->beginTransmission(addr);
    return (wire_->endTransmission() == 0);
}

bool I2cBus::readReg(uint8_t addr, uint8_t reg, uint8_t* out, size_t len) {
    if (!wire_ || !out || len == 0) return false;
    wire_->beginTransmission(addr);
    wire_->write(reg);
    if (wire_->endTransmission(false) != 0) return false;
    size_t got = wire_->requestFrom((int)addr, (int)len);
    if (got != len) return false;
    for (size_t i = 0; i < len; ++i) {
        out[i] = wire_->read();
    }
    return true;
}

bool I2cBus::writeReg(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len) {
    if (!wire_ || (!data && len > 0)) return false;
    wire_->beginTransmission(addr);
    wire_->write(reg);
    if (len > 0) wire_->write(data, len);
    return (wire_->endTransmission() == 0);
}
