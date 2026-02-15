/**
 * @file FiltrationWindow.cpp
 * @brief Deterministic filtration window computation helper implementation.
 */

#include "Modules/PoolLogicModule/FiltrationWindow.h"
#include "Domain/PoolLogicDefaults.h"
#include <math.h>

bool computeFiltrationWindowDeterministic(const FiltrationWindowInput& in, FiltrationWindowOutput& out)
{
    if (!isfinite(in.waterTemp)) return false;

    int duration = PoolDefaults::MinDurationHours;
    if (in.waterTemp < in.lowThreshold) {
        duration = PoolDefaults::MinDurationHours;
    } else if (in.waterTemp < in.setpoint) {
        duration = (int)lroundf(in.waterTemp * PoolDefaults::FactorLow);
    } else {
        duration = (int)lroundf(in.waterTemp * PoolDefaults::FactorHigh);
    }

    if (duration < PoolDefaults::MinDurationHours) duration = PoolDefaults::MinDurationHours;
    if (duration > PoolDefaults::MaxDurationHours) duration = PoolDefaults::MaxDurationHours;

    int startMin = (int)in.startMinHour;
    if (startMin < 0) startMin = 0;
    if (startMin > PoolDefaults::MaxClockHour) startMin = PoolDefaults::MaxClockHour;

    int stopMax = (int)in.stopMaxHour;
    if (stopMax > PoolDefaults::MaxClockHour) stopMax = PoolDefaults::MaxClockHour;
    if (stopMax < 0) stopMax = 0;

    int start = PoolDefaults::FiltrationPivotHour - (int)lroundf((float)duration * PoolDefaults::FactorHigh);
    if (start < startMin) start = startMin;

    int stop = start + duration;
    if (stop > stopMax) stop = stopMax;

    if (stop <= start) {
        if (start < PoolDefaults::MaxClockHour) {
            stop = start + PoolDefaults::MinEmergencyDurationHours;
        } else {
            start = PoolDefaults::FallbackStartHour;
            stop = PoolDefaults::MaxClockHour;
        }
    }

    out.durationHours = (uint8_t)(stop - start);
    out.startHour = (uint8_t)start;
    out.stopHour = (uint8_t)stop;
    return true;
}
