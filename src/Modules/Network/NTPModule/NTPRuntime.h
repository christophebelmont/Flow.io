#pragma once
/**
 * @file NTPRuntime.h
 * @brief NTP runtime helpers and keys.
 */

#include "Core/DataStore/DataStore.h"
#include "Core/EventBus/EventPayloads.h"

// RUNTIME_PUBLIC

// Data keys for NTP runtime values.
constexpr DataKey DATAKEY_TIME_READY = 3;

static inline bool timeReady(const DataStore& ds)
{
    return ds.data().ntp.timeReady;
}

static inline void setTimeReady(DataStore& ds, bool ready)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.ntp.timeReady == ready) return;
    rt.ntp.timeReady = ready;
    ds.notifyChanged(DATAKEY_TIME_READY, DIRTY_TIME);
}
