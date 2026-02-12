#pragma once
/**
 * @file GpioDriver.h
 * @brief ESP32 GPIO in/out driver.
 */

#include <stdint.h>
#include "Modules/IOModule/IODrivers/IODriver.h"

class GpioDriver : public IODriver {
public:
    enum InputPullMode : uint8_t {
        PullNone = 0,
        PullUp = 1,
        PullDown = 2
    };

    GpioDriver(const char* driverId, uint8_t pin, bool output, bool activeHigh,
               uint8_t inputPullMode = PullNone);

    const char* id() const override { return driverId_; }
    bool begin() override;
    void tick(uint32_t) override {}

    bool write(bool on);
    bool read(bool& on) const;

private:
    const char* driverId_ = nullptr;
    uint8_t pin_ = 0;
    bool output_ = false;
    bool activeHigh_ = true;
    uint8_t inputPullMode_ = PullNone;
};
