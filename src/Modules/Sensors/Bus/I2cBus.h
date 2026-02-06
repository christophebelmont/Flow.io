#pragma once
/**
 * @file I2cBus.h
 * @brief I2C (Wire) wrapper.
 */
#include <stdint.h>
#include <cstddef>
class TwoWire;

class I2cBus {
public:
    explicit I2cBus(TwoWire* wire);
    bool begin(int sda, int scl, uint32_t freqHz);

    bool probe(uint8_t addr);
    bool readReg(uint8_t addr, uint8_t reg, uint8_t* out, size_t len);
    bool writeReg(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len);

private:
    TwoWire* wire_ = nullptr;
};
