#pragma once
/**
 * @file ActuatorsRuntime.h
 * @brief Actuators runtime helpers and keys.
 */
#include "Core/DataStore/DataStore.h"
#include "Core/EventBus/EventPayloads.h"
#include "Modules/Actuators/ActuatorsModuleDataModel.h"

// RUNTIME_PUBLIC

constexpr DataKey DATAKEY_ACTUATOR_BASE = 20;

static inline ActuatorRuntime actuatorRuntime(const DataStore& ds, uint8_t idx)
{
    if (idx >= ACTUATOR_MAX) return ActuatorRuntime{};
    return ds.data().actuators.slots[idx];
}

static inline void setActuatorRuntime(DataStore& ds, uint8_t idx, const ActuatorRuntime& v)
{
    if (idx >= ACTUATOR_MAX) return;
    RuntimeData& rt = ds.dataMutable();
    ActuatorRuntime& cur = rt.actuators.slots[idx];

    if (cur.on == v.on &&
        cur.commanded == v.commanded &&
        cur.uptimeMs == v.uptimeMs &&
        cur.tankFillPct == v.tankFillPct)
        return;

    cur = v;
    ds.notifyChanged((DataKey)(DATAKEY_ACTUATOR_BASE + idx), DIRTY_ACTUATORS);
}
