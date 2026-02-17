# Modèle DataStore, EventBus et Config

## DataStore

`DataStore` contient `RuntimeData` (agrégé depuis `ModuleDataModel_Generated.h`) et publie:
- `EventId::DataChanged` (clé unitaire `DataKey`)
- `EventId::DataSnapshotAvailable` (bitmask `dirtyFlags`)

### Dirty flags (`EventPayloads.h`)

- `DIRTY_NETWORK`
- `DIRTY_TIME`
- `DIRTY_MQTT`
- `DIRTY_SENSORS`
- `DIRTY_ACTUATORS`

### DataKeys réservées (`include/Core/DataKeys.h`)

Clés fixes:
- `1`: `WifiReady`
- `2`: `WifiIp`
- `3`: `TimeReady`
- `4`: `MqttReady`
- `5`: `MqttRxDrop`
- `6`: `MqttParseFail`
- `7`: `MqttHandlerFail`
- `8`: `MqttOversizeDrop`
- `10`: `HaPublished`
- `11`: `HaVendor`
- `12`: `HaDeviceId`

Plages réservées:
- `40..63`: I/O runtime (`IoBase`, 24 endpoints)
- `80..87`: pool-device state (`PoolDeviceStateBase`)
- `88..95`: pool-device metrics (`PoolDeviceMetricsBase`)

## EventBus

### Event IDs standards (`EventId.h`)

- `SystemStarted`
- `DataChanged`
- `DataSnapshotAvailable`
- `ConfigChanged`
- `SchedulerEventTriggered`
- `AlarmRaised`
- `AlarmCleared`
- `AlarmAcked`
- `AlarmSilenceChanged`
- `AlarmConditionChanged`

Notes:
- payload max EventBus: 48 octets (`EventBus::MAX_PAYLOAD_SIZE`)
- dispatch batch: `dispatch(8)` côté `EventBusModule`

### Publishers principaux

- `ConfigStore`: publie `ConfigChanged`
- `DataStore`: publie `DataChanged` + `DataSnapshotAvailable`
- `TimeModule`: publie `SchedulerEventTriggered`
- `AlarmModule`: publie événements alarmes
- `EventBusModule`: publie `SystemStarted` au démarrage

### Subscribers principaux

- `MQTTModule`: `DataChanged`, `DataSnapshotAvailable`, `ConfigChanged`, événements alarmes
- `TimeModule`: `DataChanged`, `ConfigChanged`
- `HAModule`: `DataChanged`
- `PoolDeviceModule`: `SchedulerEventTriggered`, `DataChanged`, `ConfigChanged`
- `PoolLogicModule`: `SchedulerEventTriggered`

## ConfigStore et persistance NVS

`ConfigStore`:
- enregistre des `ConfigVariable<T>` (module/jsonName/nvsKey/type/persistence)
- charge les persistents au boot (`loadPersistent`)
- applique patch JSON (`applyJson`)
- notifie `ConfigChanged` pour chaque clé modifiée

## Branches de configuration (`ConfigBranchIds.h`)

Branches globales:
- `Wifi`, `Mqtt`, `Ha`, `Time`, `TimeScheduler`, `SystemMonitor`, `PoolLogic`, `Alarms`

Branches IO:
- `Io`, `IoDebug`
- `IoInputA0..A5`
- `IoOutputD0..D7`

Branches pool-device:
- `PoolDevicePd0..Pd7` (config métier)
- `PoolDeviceRuntimePd0..Pd7` (blob runtime persistant `metrics_blob`)

## Remarque sur “EventStore”

Le projet ne stocke pas un journal d'événements persistant dédié.
- `EventBus` est volatile et orienté dispatch temps réel.
- l'état durable est assuré par `ConfigStore` (NVS).
