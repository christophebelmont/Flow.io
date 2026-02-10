# MQTTModule

## Description generale

`MQTTModule` gere la session MQTT (connexion, subscribe, publications runtime/config), execute les commandes recues (`cmd`), applique les patchs `cfg/set` et route la telemetrie.

## Dependances module

- `moduleId`: `mqtt`
- Dependances declarees: `loghub`, `wifi`

## Services proposes

- `mqtt` -> `MqttService`
  - `publish`
  - `formatTopic`
  - `isConnected`

## Services consommes

- `wifi` (`WifiService`) pour contexte reseau
- `cmd` (`CommandService`) pour executer les commandes recues
- `config` (`ConfigStoreService`) pour `cfg/set`, snapshots `cfg/<module>`, listing modules
- `eventbus` (`EventBusService`) pour consommer:
  - `DataChanged`
  - `DataSnapshotAvailable`
  - `ConfigChanged`
- `datastore` (`DataStoreService`) pour etat reseau/mqtt et snapshots runtime
- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Module JSON: `mqtt`
- Cles:
  - `host`
  - `port`
  - `user`
  - `pass`
  - `baseTopic`
  - `enabled`
  - `sensor_min_publish_ms`

## Valeurs DataStore utilisees

- Ecrit:
  - `mqtt.mqttReady` (DataKey `DATAKEY_MQTT_READY` = 4, dirty `DIRTY_MQTT`)
- Lu:
  - `wifi.ready` pour le warmup reseau avant connexion
  - runtime global via `DataStore*` pour les publishers de snapshots
