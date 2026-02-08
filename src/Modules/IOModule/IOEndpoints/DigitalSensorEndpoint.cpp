/**
 * @file DigitalSensorEndpoint.cpp
 * @brief Implementation file.
 */

#include "DigitalSensorEndpoint.h"

DigitalSensorEndpoint::DigitalSensorEndpoint(const char* endpointId)
    : endpointId_(endpointId)
{
    value_.valueType = IO_EP_VALUE_BOOL;
    value_.v.b = false;
    value_.valid = false;
    value_.timestampMs = 0;
}

bool DigitalSensorEndpoint::read(IOEndpointValue& out)
{
    out = value_;
    return value_.valid;
}

void DigitalSensorEndpoint::update(bool on, bool valid, uint32_t timestampMs)
{
    value_.valueType = IO_EP_VALUE_BOOL;
    value_.v.b = on;
    value_.valid = valid;
    value_.timestampMs = timestampMs;
}
