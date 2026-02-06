#include "ActuatorVirtualDriver.h"
#include <Arduino.h>

SensorReading ActuatorVirtualDriver::read()
{
    SensorReading r;
    r.timestampMs = millis();
    if (!ds_) {
        r.valid = false;
        r.value = 0.0f;
        return r;
    }

    ActuatorRuntime rt = actuatorRuntime(*ds_, idx_);
    r.valid = true;
    if (metric_ == Metric::TankFillPct) {
        r.value = rt.tankFillPct;
    } else {
        r.value = (float)rt.uptimeMs / 3600000.0f;
    }
    return r;
}
