/**
 * @file AdcBus.cpp
 * @brief Implementation file.
 */
#include "AdcBus.h"
#include <Arduino.h>

void AdcBus::begin(uint8_t resolutionBits, int attenuation) {
    resolution_ = resolutionBits;
    attenuation_ = attenuation;
    analogReadResolution(resolution_);
    analogSetAttenuation((adc_attenuation_t)attenuation_);
}

int AdcBus::read(int pin) const {
    return analogRead(pin);
}
