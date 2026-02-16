# DataStore

## Purpose

DataStore provides a shared runtime state container (`RuntimeData`) and a notification mechanism for changes.

Modules typically:
- write to `ds.dataMutable()`
- call `notifyChanged(DataKey key, DirtyFlags flags)`
- optionally publish snapshots through `RuntimeSnapshotProvider`

## Runtime Data Model

`RuntimeData` is generated at build time and aggregates per-module structs (e.g. `WifiRuntimeData`, `MQTTRuntimeData`, `IORuntimeData`, ...).

```cpp
RuntimeData& rt = ds.dataMutable();
rt.mqtt.mqttReady = true;
ds.notifyChanged(DataKeys::MqttReady, DIRTY_MQTT);
```

## DataKeys

`DataKeys` defines stable identifiers (`DataKey`) for runtime fields.
Modules generally expose convenience helpers in their `*Runtime.h` headers.

Example: MQTT runtime helpers (`Modules/Network/MQTTModule/MQTTRuntime.h`).

## Change Notifications

A module can subscribe to changes via EventBus events using `DataChangedPayload`:
- `EventType::DataChanged`
- payload contains `DataKey id`

## Snapshot Model

Some modules expose a set of snapshots that can be routed to MQTT publishers.

Pattern (IOModule):
```cpp
uint8_t runtimeSnapshotCount() const;
const char* runtimeSnapshotSuffix(uint8_t idx) const;
bool buildRuntimeSnapshot(uint8_t idx, char* out, size_t len, uint32_t& maxTsOut) const;
```
