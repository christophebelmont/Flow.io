# MQTTModule (`moduleId: mqtt`)

## Rôle

Gateway MQTT principal:
- connexion broker, reconnect/backoff
- souscription commandes (`cmd`) et patch config (`cfg/set`)
- publication ACK, status, config, runtime snapshots, alarmes
- exposition de `MqttService`

Type: module actif.

## Dépendances

- `loghub`
- `wifi`
- `cmd`
- `time`
- `alarms`

## Affinité / cadence

- core: 0
- task: `mqtt`
- loop delay: `Limits::Mqtt::Timing::LoopDelayMs` (50ms)

## Services exposés

- `mqtt` -> `MqttService`
  - `publish`
  - `formatTopic`
  - `isConnected`

## Services consommés

- `wifi` (`WifiService`)
- `cmd` (`CommandService`)
- `config` (`ConfigStoreService`)
- `time.scheduler` (`TimeSchedulerService`)
- `alarms` (`AlarmService`)
- `eventbus` (`EventBusService`)
- `datastore` (`DataStoreService`)

## Config / NVS

Branche `ConfigBranchId::Mqtt` (`module: mqtt`):
- `host`, `port`, `user`, `pass`, `baseTopic`, `enabled`, `sens_min_pub_ms`

## Topics majeurs

- `<base>/<device>/cmd` (RX)
- `<base>/<device>/ack` (TX)
- `<base>/<device>/status` (TX + LWT)
- `<base>/<device>/cfg/set` (RX)
- `<base>/<device>/cfg/ack` (TX)
- `<base>/<device>/cfg/<module>` (TX)
- `<base>/<device>/rt/...` (TX runtime)

Détail complet: `../core/mqtt-topics.md`

## Command path (`cmd`)

Entrée JSON attendue:
```json
{"cmd":"...","args":{...}}
```
Traitement:
1. parse JSON
2. `cmdSvc->execute(...)`
3. publish ACK sur `ack`

## Config path (`cfg/set`)

Entrée JSON patch multi-modules.
Traitement:
1. validation JSON
2. `cfgSvc->applyJson(...)`
3. ACK sur `cfg/ack`
4. publication config ciblée pilotée par événements `ConfigChanged`

## EventBus

Abonnements:
- `DataChanged`
- `DataSnapshotAvailable`
- `ConfigChanged`
- `AlarmRaised`, `AlarmCleared`, `AlarmAcked`, `AlarmSilenceChanged`, `AlarmConditionChanged`

Effets:
- `DataSnapshotAvailable`: déclenche publication runtime sélective (dirty flags)
- `ConfigChanged`: publication `cfg/<module>` ciblée, reconnect si clés MQTT de connexion modifiées
- événements alarmes: publication `rt/alarms/id*` + `rt/alarms/m`

## DataStore

Écritures via `MQTTRuntime.h`:
- `MqttReady` (notifié)
- compteurs RX (`rxDrop`, `oversizeDrop`, `parseFail`, `handlerFail`) mis à jour en continu

## Publication runtime

- sensors publisher (mux runtime depuis `main.cpp`)
- publishers périodiques additionnels (`addRuntimePublisher`)
- throttling via `sensorMinPublishMs`

## Points clés robustesse

- backoff reconnect avec jitter
- queue RX bornée
- garde-fous overflow topic/payload
- JSON statiques (ArduinoJson documents bornés)
