/**
 * @file Pcf8574Driver.cpp
 * @brief Implementation file.
 */

#include "Pcf8574Driver.h"

Pcf8574Driver::Pcf8574Driver(const char* driverId, I2CBus* bus, uint8_t address)
    : driverId_(driverId), bus_(bus), address_(address)
{
}

bool Pcf8574Driver::begin()
{
    return flush_();
}

bool Pcf8574Driver::writeMask(uint8_t mask)
{
    state_ = mask;
    return flush_();
}

bool Pcf8574Driver::readMask(uint8_t& mask) const
{
    mask = state_;
    return true;
}

bool Pcf8574Driver::writePin(uint8_t pin, bool on)
{
    if (pin > 7) return false;

    if (on) state_ |= (uint8_t)(1u << pin);
    else state_ &= (uint8_t)~(1u << pin);

    return flush_();
}

bool Pcf8574Driver::readShadow(uint8_t pin, bool& on) const
{
    if (pin > 7) return false;
    on = (state_ & (uint8_t)(1u << pin)) != 0;
    return true;
}

bool Pcf8574Driver::flush_()
{
    if (!bus_) return false;
    if (!bus_->lock(20)) return false;
    bool ok = bus_->writeBytes(address_, &state_, 1);
    bus_->unlock();
    return ok;
}
