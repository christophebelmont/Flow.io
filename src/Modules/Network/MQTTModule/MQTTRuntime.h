#pragma once
/**
 * @file MQTTRuntime.h
 * @brief MQTT runtime helpers and keys.
 */

#include "Core/DataStore/DataStore.h"
#include "Core/EventBus/EventPayloads.h"

// RUNTIME_PUBLIC

// Data keys for MQTT runtime values.
constexpr DataKey DATAKEY_MQTT_READY = 4;

static inline bool mqttReady(const DataStore& ds)
{
    return ds.data().mqtt.mqttReady;
}

static inline void setMqttReady(DataStore& ds, bool ready)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.mqtt.mqttReady == ready) return;
    rt.mqtt.mqttReady = ready;
    ds.notifyChanged(DATAKEY_MQTT_READY, DIRTY_MQTT);
}
