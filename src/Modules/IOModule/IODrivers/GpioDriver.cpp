/**
 * @file GpioDriver.cpp
 * @brief Implementation file.
 */

#include "GpioDriver.h"
#include <Arduino.h>

GpioDriver::GpioDriver(const char* driverId, uint8_t pin, bool output, bool activeHigh)
    : driverId_(driverId), pin_(pin), output_(output), activeHigh_(activeHigh)
{
}

bool GpioDriver::begin()
{
    pinMode(pin_, output_ ? OUTPUT : INPUT);
    if (output_) {
        digitalWrite(pin_, activeHigh_ ? LOW : HIGH);
    }
    return true;
}

bool GpioDriver::write(bool on)
{
    if (!output_) return false;
    bool level = on ? activeHigh_ : !activeHigh_;
    digitalWrite(pin_, level ? HIGH : LOW);
    return true;
}

bool GpioDriver::read(bool& on) const
{
    int level = digitalRead(pin_);
    on = activeHigh_ ? (level == HIGH) : (level == LOW);
    return true;
}
