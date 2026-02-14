#pragma once

#include <stdint.h>

namespace PoolDefaults {

constexpr uint8_t FiltrationPivotHour = 15;
constexpr uint8_t MinDurationHours = 2;
constexpr uint8_t MaxDurationHours = 24;
constexpr uint8_t MaxClockHour = 23;
constexpr uint8_t FallbackStartHour = 22;
constexpr uint8_t MinEmergencyDurationHours = 1;

constexpr float TempLow = 12.0f;
constexpr float TempHigh = 24.0f;
constexpr float FactorLow = 1.0f / 3.0f;
constexpr float FactorHigh = 1.0f / 2.0f;

constexpr uint8_t FiltrationStartMinHour = 8;
constexpr uint8_t FiltrationStopMaxHour = 23;

}  // namespace PoolDefaults
