#pragma once
/**
 * @file Tca9554Driver.h
 * @brief TCA9554 output-only driver.
 */

#include <stdint.h>
#include "Modules/IOModule/IOBus/I2CBus.h"
#include "Modules/IOModule/IODrivers/IODriver.h"

class Tca9554Driver : public IMaskOutputDriver {
public:
    Tca9554Driver(const char* driverId, I2CBus* bus, uint8_t address);

    const char* id() const override { return driverId_; }
    bool begin() override;
    void tick(uint32_t) override {}

    bool writeMask(uint8_t mask) override;
    bool readMask(uint8_t& mask) const override;
    bool writePin(uint8_t pin, bool on);
    bool readShadow(uint8_t pin, bool& on) const;

private:
    bool writeReg_(uint8_t reg, uint8_t value);

    static constexpr uint8_t kRegOutputPort = 0x01;
    static constexpr uint8_t kRegPolarityInversion = 0x02;
    static constexpr uint8_t kRegConfiguration = 0x03;

    const char* driverId_ = nullptr;
    I2CBus* bus_ = nullptr;
    uint8_t address_ = 0x20;
    uint8_t state_ = 0x00;
};
