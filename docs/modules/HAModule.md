# HAModule

## Description generale

`HAModule` publie l'auto-decouverte Home Assistant (entites capteurs/actionneurs/config), sans bloquer l'EventBus, a partir des schemas `ConfigStore` et des etats runtime.

## Dependances module

- `moduleId`: `ha`
- Dependances declarees: `eventbus`, `config`, `datastore`, `mqtt`

## Services proposes

- Aucun service expose.

## Services consommes

- `eventbus` (`EventBusService`) pour consommer `DataChanged`
- `config` (`ConfigStoreService`) pour introspection des blocs de config
- `datastore` (`DataStoreService`) pour lire et ecrire l'etat HA runtime
- `mqtt` (`MqttService`) pour publier les topics discovery

## Valeurs ConfigStore utilisees

- Module JSON: `ha`
- Cles:
  - `enabled`
  - `vendor`
  - `device_id`
  - `discovery_prefix`
  - `model`

## Valeurs DataStore utilisees

- Ecrit:
  - `ha.autoconfigPublished` (DataKey `DATAKEY_HA_PUBLISHED` = 10, dirty `DIRTY_MQTT`)
  - `ha.vendor` (DataKey `DATAKEY_HA_VENDOR` = 11, dirty `DIRTY_MQTT`)
  - `ha.deviceId` (DataKey `DATAKEY_HA_DEVICE_ID` = 12, dirty `DIRTY_MQTT`)
- Lu:
  - `wifi.ready`
  - `mqtt.mqttReady`
  - `pool.devices[i].valid` (selection `pool.write` vs `io.write` pour les switches)
