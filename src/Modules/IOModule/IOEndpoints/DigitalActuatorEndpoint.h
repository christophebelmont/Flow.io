#pragma once
/**
 * @file DigitalActuatorEndpoint.h
 * @brief Typed bool actuator endpoint.
 */

#include "Modules/IOModule/IOEndpoints/IOEndpoint.h"

typedef bool (*DigitalWriteFn)(void* ctx, bool on);

class DigitalActuatorEndpoint : public IOEndpoint {
public:
    DigitalActuatorEndpoint(const char* endpointId, DigitalWriteFn writeFn, void* writeCtx);

    const char* id() const override { return endpointId_; }
    IOEndpointType type() const override { return IO_EP_DIGITAL_ACTUATOR; }
    uint8_t capabilities() const override { return IO_CAP_READ | IO_CAP_WRITE; }

    bool read(IOEndpointValue& out) override;
    bool write(const IOEndpointValue& in) override;

private:
    const char* endpointId_ = nullptr;
    DigitalWriteFn writeFn_ = nullptr;
    void* writeCtx_ = nullptr;
    IOEndpointValue value_{};
};
