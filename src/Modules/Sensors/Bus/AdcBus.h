#pragma once
/**
 * @file AdcBus.h
 * @brief ESP32 ADC wrapper.
 */
#include <stdint.h>

class AdcBus {
public:
    void begin(uint8_t resolutionBits, int attenuation);
    int read(int pin) const;

private:
    uint8_t resolution_ = 12;
    int attenuation_ = 0;
};
