# CommandModule (`moduleId: cmd`)

## Rôle

Expose la registry de commandes (`CommandRegistry`) au reste du système.

Type: module passif.

## Dépendances

- `loghub`

## Services exposés

- `cmd` -> `CommandService`
  - `registerHandler`
  - `execute`

## Services consommés

- `loghub` (log init)

## Config / NVS

Aucune variable `ConfigStore`.

## EventBus / DataStore / MQTT

Aucun direct.

## Commandes enregistrées dans le système (autres modules)

Le `CommandModule` n'ajoute pas de commande métier lui-même; il sert d'infrastructure.
Commandes enregistrées par d'autres modules:
- `system.ping`, `system.reboot`, `system.factory_reset`
- `alarms.list`, `alarms.ack`, `alarms.ack_all`
- `time.resync`, `ntp.resync`
- `time.scheduler.info/get/set/clear/clear_all`
- `poollogic.filtration.write`, `poollogic.filtration.recalc`, `poollogic.auto_mode.set`
- `pooldevice.write`, `pool.write`, `pool.refill`
