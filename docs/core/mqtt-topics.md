# Topologie MQTT détaillée

Base topic calculé:
- `<baseTopic>/<deviceId>/...`
- `baseTopic`: config `mqtt.baseTopic` (défaut `flowio`)
- `deviceId`: `ESP32-XXXXXX` (suffix MAC)

## Channels de contrôle

- `cmd`: `<base>/<device>/cmd`
- `ack`: `<base>/<device>/ack`
- `status`: `<base>/<device>/status` (LWT + online)
- `cfg/set`: `<base>/<device>/cfg/set`
- `cfg/ack`: `<base>/<device>/cfg/ack`

### Souscriptions

`MQTTModule` subscribe à:
- `cmd` (QoS 0)
- `cfg/set` (QoS 1)

### Publications état

- `status` retain=true:
  - online: `{"online":true}`
  - LWT offline: `{"online":false}`

## Publication configuration (`cfg/*`)

### Règle générale

Pour chaque module de `ConfigStore::listModules()`:
- topic: `<base>/<device>/cfg/<module>`
- payload: JSON module (`toJsonModule`)

### Cas spécial `time/scheduler`

- root: `<base>/<device>/cfg/time/scheduler`
  - payload: `{"mode":"per_slot","slots":16}`
- détail slot: `<base>/<device>/cfg/time/scheduler/slotN`
  - payload complet du slot (used/event_id/mode/start/end...)

### Déclenchement

- publication initiale/ramp à la connexion MQTT
- publication incrémentale par `ConfigChanged` (branch routing)
- reconnect MQTT si changement de clé de connexion (`host`, `port`, `user`, `pass`, `baseTopic`)

## Publication runtime (`rt/*`)

### Multiplex runtime (depuis `main.cpp`)

Routage snapshots providers:
- `rt/io/input/aN`
- `rt/io/input/iN`
- `rt/io/output/dN`
- `rt/pdm/state/pdN`
- `rt/pdm/metrics/pdN`

Le callback multiplex (`rt/runtime/mux`) publie directement les routes détaillées et ne publie pas de payload sur son topic lié.

### Snapshots périodiques additionnels

- `<base>/<device>/rt/runtime/state` (30s)
- `<base>/<device>/rt/network/state` (60s)
- `<base>/<device>/rt/system/state` (60s)

### Alarmes

- meta: `<base>/<device>/rt/alarms/m`
- par id: `<base>/<device>/rt/alarms/id<alarmId>`

## Débit et throttling

- `sensorMinPublishMs` (config MQTT) limite la fréquence de publication des snapshots runtime.
- Dirty flags `DIRTY_SENSORS`/`DIRTY_ACTUATORS` pilotent la publication sélective.
- Les routes d'actionneurs peuvent être forcées pendant la fenêtre startup/reconnect pour éviter un état domotique stale.

## Commandes MQTT (payload type)

### `cmd`

Attendu:
```json
{"cmd":"pooldevice.write","args":{"slot":1,"value":true}}
```
Réponse sur `ack`:
- succès: `{"ok":true,"cmd":"...","reply":{...}}`
- erreur: JSON d'erreur standard (`ErrorCode`)

### `cfg/set`

Attendu (patch config multi-modules):
```json
{"poollogic":{"auto_mode":true},"pdm/pd1":{"flow_l_h":1.4}}
```
Réponse sur `cfg/ack`:
- succès: `{"ok":true,...}`
- échec: JSON d'erreur standard
