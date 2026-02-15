/**
 * @file Ds18b20Driver.cpp
 * @brief Implementation file.
 */

#include "Ds18b20Driver.h"
#include <DallasTemperature.h>

Ds18b20Driver::Ds18b20Driver(const char* driverId, OneWireBus* bus, const uint8_t address[8],
                             const Ds18b20DriverConfig& cfg)
    : driverId_(driverId), bus_(bus), cfg_(cfg)
{
    if (address) memcpy(address_, address, sizeof(address_));
}

bool Ds18b20Driver::begin()
{
    if (!bus_) return false;
    bus_->begin();
    bus_->setWaitForConversion(false);
    requested_ = false;
    valid_ = false;
    return true;
}

void Ds18b20Driver::tick(uint32_t nowMs)
{
    if (!bus_) return;

    if (!requested_) {
        bus_->request();
        lastRequestMs_ = nowMs;
        requested_ = true;
        return;
    }

    if ((uint32_t)(nowMs - lastRequestMs_) < cfg_.conversionWaitMs) return;

    float c = bus_->readC(address_);
    if (c != DEVICE_DISCONNECTED_C) {
        celsius_ = c;
        valid_ = true;
    }

    if ((uint32_t)(nowMs - lastRequestMs_) >= cfg_.pollMs) {
        bus_->request();
        lastRequestMs_ = nowMs;
    }
}

bool Ds18b20Driver::readCelsius(float& out) const
{
    if (!valid_) return false;
    out = celsius_;
    return true;
}

bool Ds18b20Driver::readSample(uint8_t, IOAnalogSample& out) const
{
    float c = 0.0f;
    if (!readCelsius(c)) return false;
    out = IOAnalogSample{};
    out.value = c;
    return true;
}
