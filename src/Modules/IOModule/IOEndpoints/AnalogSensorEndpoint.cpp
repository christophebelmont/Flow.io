/**
 * @file AnalogSensorEndpoint.cpp
 * @brief Implementation file.
 */

#include "AnalogSensorEndpoint.h"

AnalogSensorEndpoint::AnalogSensorEndpoint(const char* endpointId)
    : endpointId_(endpointId)
{
    value_.valueType = IO_EP_VALUE_FLOAT;
    value_.v.f = 0.0f;
    value_.valid = false;
    value_.timestampMs = 0;
}

bool AnalogSensorEndpoint::read(IOEndpointValue& out)
{
    out = value_;
    return value_.valid;
}

void AnalogSensorEndpoint::update(float value, bool valid, uint32_t timestampMs)
{
    value_.valueType = IO_EP_VALUE_FLOAT;
    value_.v.f = value;
    value_.valid = valid;
    value_.timestampMs = timestampMs;
}
