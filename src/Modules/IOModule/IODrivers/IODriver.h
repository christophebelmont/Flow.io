#pragma once
/**
 * @file IODriver.h
 * @brief Base interface for IO drivers.
 */

#include <stdint.h>

class IODriver {
public:
    virtual ~IODriver() = default;
    virtual const char* id() const = 0;
    virtual bool begin() = 0;
    virtual void tick(uint32_t nowMs) = 0;
};

struct IOAnalogSample {
    float value = 0.0f;
    int16_t raw = 0;
    uint32_t seq = 0;
    bool hasRaw = false;
    bool hasSeq = false;
};

class IDigitalPinDriver : public IODriver {
public:
    virtual bool write(bool on) = 0;
    virtual bool read(bool& on) const = 0;
};

class IAnalogSourceDriver : public IODriver {
public:
    virtual bool readSample(uint8_t channel, IOAnalogSample& out) const = 0;
};

class IMaskOutputDriver : public IODriver {
public:
    virtual bool writeMask(uint8_t mask) = 0;
    virtual bool readMask(uint8_t& mask) const = 0;
};
