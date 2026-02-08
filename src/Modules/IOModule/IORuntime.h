#pragma once
/**
 * @file IORuntime.h
 * @brief IO runtime helpers and keys.
 */

#include <math.h>
#include <stdint.h>
#include "Core/DataStore/DataStore.h"
#include "Core/EventBus/EventPayloads.h"

// RUNTIME_PUBLIC

constexpr DataKey DATAKEY_IO_BASE = 40;
constexpr DataKey DATAKEY_IO_PH = 5;
constexpr DataKey DATAKEY_IO_ORP = 6;
constexpr DataKey DATAKEY_IO_PSI = 7;
constexpr DataKey DATAKEY_IO_WATER_TEMP = 8;
constexpr DataKey DATAKEY_IO_AIR_TEMP = 9;

static inline float ioRoundToPrecision(float value, int32_t decimals)
{
    if (decimals <= 0) return (float)((int32_t)lroundf(value));
    float scale = 1.0f;
    for (int32_t i = 0; i < decimals; ++i) scale *= 10.0f;
    return roundf(value * scale) / scale;
}

static inline bool ioChangedAtPrecision(float a, float b, int32_t decimals)
{
    return ioRoundToPrecision(a, decimals) != ioRoundToPrecision(b, decimals);
}

static inline float ioPh(const DataStore& ds) { return ds.data().io.ph; }
static inline float ioOrp(const DataStore& ds) { return ds.data().io.orp; }
static inline float ioPsi(const DataStore& ds) { return ds.data().io.psi; }
static inline float ioWaterTemp(const DataStore& ds) { return ds.data().io.waterTemp; }
static inline float ioAirTemp(const DataStore& ds) { return ds.data().io.airTemp; }

static inline void setIoPh(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.io.ph == value) return;
    rt.io.ph = value;
    ds.notifyChanged(DATAKEY_IO_PH, DIRTY_SENSORS);
}

static inline void setIoOrp(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.io.orp == value) return;
    rt.io.orp = value;
    ds.notifyChanged(DATAKEY_IO_ORP, DIRTY_SENSORS);
}

static inline void setIoPsi(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.io.psi == value) return;
    rt.io.psi = value;
    ds.notifyChanged(DATAKEY_IO_PSI, DIRTY_SENSORS);
}

static inline void setIoWaterTemp(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.io.waterTemp == value) return;
    rt.io.waterTemp = value;
    ds.notifyChanged(DATAKEY_IO_WATER_TEMP, DIRTY_SENSORS);
}

static inline void setIoAirTemp(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.io.airTemp == value) return;
    rt.io.airTemp = value;
    ds.notifyChanged(DATAKEY_IO_AIR_TEMP, DIRTY_SENSORS);
}
