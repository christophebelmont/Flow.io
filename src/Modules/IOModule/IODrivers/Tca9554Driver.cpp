/**
 * @file Tca9554Driver.cpp
 * @brief Implementation file.
 */

#include "Tca9554Driver.h"

Tca9554Driver::Tca9554Driver(const char* driverId, I2CBus* bus, uint8_t address)
    : driverId_(driverId), bus_(bus), address_(address)
{
}

bool Tca9554Driver::begin()
{
    // Output-only mode. Write output register BEFORE switching to output mode
    // to avoid a glitch: the TCA9554 POR output register is 0xFF, so configuring
    // pins as outputs first would briefly drive them HIGH before state_ is applied.
    if (!writeReg_(kRegPolarityInversion, 0x00)) return false;
    if (!writeReg_(kRegOutputPort, state_)) return false;
    return writeReg_(kRegConfiguration, 0x00);
}

bool Tca9554Driver::writeMask(uint8_t mask)
{
    state_ = mask;
    return writeReg_(kRegOutputPort, state_);
}

bool Tca9554Driver::readMask(uint8_t& mask) const
{
    mask = state_;
    return true;
}

bool Tca9554Driver::writePin(uint8_t pin, bool on)
{
    if (pin > 7) return false;

    if (on) state_ |= (uint8_t)(1u << pin);
    else state_ &= (uint8_t)~(1u << pin);

    return writeReg_(kRegOutputPort, state_);
}

bool Tca9554Driver::readShadow(uint8_t pin, bool& on) const
{
    if (pin > 7) return false;
    on = (state_ & (uint8_t)(1u << pin)) != 0;
    return true;
}

bool Tca9554Driver::writeReg_(uint8_t reg, uint8_t value)
{
    if (!bus_) return false;
    if (!bus_->lock(20)) return false;
    const bool ok = bus_->writeReg(address_, reg, &value, 1);
    bus_->unlock();
    return ok;
}
