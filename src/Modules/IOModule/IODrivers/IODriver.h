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
