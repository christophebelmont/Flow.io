/**
 * @file Pcf8574MaskEndpoint.cpp
 * @brief Implementation file.
 */

#include "Pcf8574MaskEndpoint.h"

Pcf8574MaskEndpoint::Pcf8574MaskEndpoint(const char* endpointId, MaskWriteFn writeFn, MaskReadFn readFn, void* fnCtx)
    : endpointId_(endpointId), writeFn_(writeFn), readFn_(readFn), fnCtx_(fnCtx)
{
}

bool Pcf8574MaskEndpoint::read(IOEndpointValue& out)
{
    uint8_t mask = 0;
    if (readFn_ && readFn_(fnCtx_, &mask)) {
        cachedMask_ = mask;
        cachedValid_ = true;
    }

    out.valueType = IO_EP_VALUE_INT32;
    out.valid = cachedValid_;
    out.timestampMs = tsMs_;
    out.v.i = (int32_t)cachedMask_;
    return out.valid;
}

bool Pcf8574MaskEndpoint::write(const IOEndpointValue& in)
{
    if (in.valueType != IO_EP_VALUE_INT32) return false;
    return setMask((uint8_t)(in.v.i & 0xFF), in.timestampMs);
}

bool Pcf8574MaskEndpoint::setMask(uint8_t mask, uint32_t tsMs)
{
    if (!writeFn_) return false;
    if (!writeFn_(fnCtx_, mask)) return false;

    cachedMask_ = mask;
    cachedValid_ = true;
    tsMs_ = tsMs;
    return true;
}

bool Pcf8574MaskEndpoint::turnOn(uint8_t bit, uint32_t tsMs)
{
    if (bit > 7) return false;
    uint8_t next = cachedMask_ | (uint8_t)(1u << bit);
    return setMask(next, tsMs);
}

bool Pcf8574MaskEndpoint::turnOff(uint8_t bit, uint32_t tsMs)
{
    if (bit > 7) return false;
    uint8_t next = cachedMask_ & (uint8_t)~(1u << bit);
    return setMask(next, tsMs);
}

bool Pcf8574MaskEndpoint::getMask(uint8_t& outMask) const
{
    if (!cachedValid_) return false;
    outMask = cachedMask_;
    return true;
}
