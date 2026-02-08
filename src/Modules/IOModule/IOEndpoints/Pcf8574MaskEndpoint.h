#pragma once
/**
 * @file Pcf8574MaskEndpoint.h
 * @brief Endpoint for full 8-bit LED mask control.
 */

#include "Modules/IOModule/IOEndpoints/IOEndpoint.h"

typedef bool (*MaskWriteFn)(void* ctx, uint8_t mask);
typedef bool (*MaskReadFn)(void* ctx, uint8_t* mask);

class Pcf8574MaskEndpoint : public IOEndpoint {
public:
    Pcf8574MaskEndpoint(const char* endpointId, MaskWriteFn writeFn, MaskReadFn readFn, void* fnCtx);

    const char* id() const override { return endpointId_; }
    IOEndpointType type() const override { return IO_EP_DIGITAL_ACTUATOR; }
    uint8_t capabilities() const override { return IO_CAP_READ | IO_CAP_WRITE; }

    bool read(IOEndpointValue& out) override;
    bool write(const IOEndpointValue& in) override;

    bool setMask(uint8_t mask, uint32_t tsMs = 0);
    bool turnOn(uint8_t bit, uint32_t tsMs = 0);
    bool turnOff(uint8_t bit, uint32_t tsMs = 0);
    bool getMask(uint8_t& outMask) const;

private:
    const char* endpointId_ = nullptr;
    MaskWriteFn writeFn_ = nullptr;
    MaskReadFn readFn_ = nullptr;
    void* fnCtx_ = nullptr;
    uint8_t cachedMask_ = 0;
    bool cachedValid_ = false;
    uint32_t tsMs_ = 0;
};
