#pragma once
/**
 * @file IOEndpoint.h
 * @brief Unified endpoint interfaces.
 */

#include <stdint.h>

enum IOEndpointType : uint8_t {
    IO_EP_ANALOG_SENSOR = 0,
    IO_EP_DIGITAL_SENSOR = 1,
    IO_EP_DIGITAL_ACTUATOR = 2
};

enum IOEndpointValueType : uint8_t {
    IO_EP_VALUE_BOOL = 0,
    IO_EP_VALUE_FLOAT = 1,
    IO_EP_VALUE_INT32 = 2
};

enum IOEndpointCapability : uint8_t {
    IO_CAP_READ = 1,
    IO_CAP_WRITE = 2
};

struct IOEndpointValue {
    uint32_t timestampMs = 0;
    bool valid = false;
    uint8_t valueType = IO_EP_VALUE_FLOAT;
    union {
        bool b;
        float f;
        int32_t i;
    } v;
};

class IOEndpoint {
public:
    virtual ~IOEndpoint() = default;
    virtual const char* id() const = 0;
    virtual IOEndpointType type() const = 0;
    virtual uint8_t capabilities() const = 0;
    virtual bool read(IOEndpointValue& out) = 0;
    virtual bool write(const IOEndpointValue& in) = 0;
};
