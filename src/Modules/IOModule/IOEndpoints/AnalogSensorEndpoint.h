#pragma once
/**
 * @file AnalogSensorEndpoint.h
 * @brief Typed float sensor endpoint.
 */

#include "Modules/IOModule/IOEndpoints/IOEndpoint.h"

class AnalogSensorEndpoint : public IOEndpoint {
public:
    explicit AnalogSensorEndpoint(const char* endpointId);

    const char* id() const override { return endpointId_; }
    IOEndpointType type() const override { return IO_EP_ANALOG_SENSOR; }
    uint8_t capabilities() const override { return IO_CAP_READ; }

    bool read(IOEndpointValue& out) override;
    bool write(const IOEndpointValue&) override { return false; }

    void update(float value, bool valid, uint32_t timestampMs);

private:
    const char* endpointId_ = nullptr;
    IOEndpointValue value_{};
};
