#pragma once
/**
 * @file Ds18b20Driver.h
 * @brief DS18B20 async request/read driver.
 */

#include <stdint.h>
#include <string.h>
#include "Modules/IOModule/IOBus/OneWireBus.h"
#include "Modules/IOModule/IODrivers/IODriver.h"

struct Ds18b20DriverConfig {
    uint32_t pollMs = 2000;
    uint32_t conversionWaitMs = 750;
};

class Ds18b20Driver : public IODriver {
public:
    Ds18b20Driver(const char* driverId, OneWireBus* bus, const uint8_t address[8],
                  const Ds18b20DriverConfig& cfg);

    const char* id() const override { return driverId_; }
    bool begin() override;
    void tick(uint32_t nowMs) override;

    bool readCelsius(float& out) const;

private:
    const char* driverId_ = nullptr;
    OneWireBus* bus_ = nullptr;
    uint8_t address_[8] = {0};
    Ds18b20DriverConfig cfg_{};

    uint32_t lastRequestMs_ = 0;
    bool requested_ = false;
    bool valid_ = false;
    float celsius_ = 0.0f;
};
