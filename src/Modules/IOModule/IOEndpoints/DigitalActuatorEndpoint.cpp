/**
 * @file DigitalActuatorEndpoint.cpp
 * @brief Implementation file.
 */

#include "DigitalActuatorEndpoint.h"

DigitalActuatorEndpoint::DigitalActuatorEndpoint(const char* endpointId, DigitalWriteFn writeFn, void* writeCtx)
    : endpointId_(endpointId), writeFn_(writeFn), writeCtx_(writeCtx)
{
    value_.valueType = IO_EP_VALUE_BOOL;
    value_.v.b = false;
    value_.valid = true;
    value_.timestampMs = 0;
}

bool DigitalActuatorEndpoint::read(IOEndpointValue& out)
{
    out = value_;
    return true;
}

bool DigitalActuatorEndpoint::write(const IOEndpointValue& in)
{
    if (in.valueType != IO_EP_VALUE_BOOL) return false;
    if (!writeFn_) return false;

    bool ok = writeFn_(writeCtx_, in.v.b);
    if (!ok) return false;

    value_.valueType = IO_EP_VALUE_BOOL;
    value_.v.b = in.v.b;
    value_.valid = true;
    value_.timestampMs = in.timestampMs;
    return true;
}
