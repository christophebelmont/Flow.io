#pragma once
/**
 * @file OneWireBus.h
 * @brief OneWire + DallasTemperature wrapper.
 */

#include <stdint.h>
#include <stddef.h>

class OneWire;
class DallasTemperature;

class OneWireBus {
public:
    explicit OneWireBus(int pin);
    ~OneWireBus();

    void begin();
    void request();
    void setWaitForConversion(bool enabled);
    bool getAddress(uint8_t index, uint8_t out[8]) const;
    float readC(const uint8_t addr[8]) const;
    uint8_t deviceCount() const;

private:
    int pin_ = -1;
    OneWire* oneWire_ = nullptr;
    DallasTemperature* dt_ = nullptr;
};
