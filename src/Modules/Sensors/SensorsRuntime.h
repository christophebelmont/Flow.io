#pragma once
/**
 * @file SensorsRuntime.h
 * @brief Sensors runtime helpers and keys.
 */
#include "Core/DataStore/DataStore.h"
#include "Core/EventBus/EventPayloads.h"

// RUNTIME_PUBLIC

// Data keys for sensors runtime values.
constexpr DataKey DATAKEY_SENSORS_PH = 5;
constexpr DataKey DATAKEY_SENSORS_ORP = 6;
constexpr DataKey DATAKEY_SENSORS_PSI = 7;

static inline float sensorsPh(const DataStore& ds)
{
    return ds.data().sensors.ph;
}

static inline float sensorsOrp(const DataStore& ds)
{
    return ds.data().sensors.orp;
}

static inline float sensorsPsi(const DataStore& ds)
{
    return ds.data().sensors.psi;
}

static inline void setSensorsPh(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.sensors.ph == value) return;
    rt.sensors.ph = value;
    ds.notifyChanged(DATAKEY_SENSORS_PH, DIRTY_SENSORS);
}

static inline void setSensorsOrp(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.sensors.orp == value) return;
    rt.sensors.orp = value;
    ds.notifyChanged(DATAKEY_SENSORS_ORP, DIRTY_SENSORS);
}

static inline void setSensorsPsi(DataStore& ds, float value)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.sensors.psi == value) return;
    rt.sensors.psi = value;
    ds.notifyChanged(DATAKEY_SENSORS_PSI, DIRTY_SENSORS);
}
