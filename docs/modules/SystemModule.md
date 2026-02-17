# SystemModule (`moduleId: system`)

## Rôle

Expose les commandes système transverses.

Type: module passif.

## Dépendances

- `loghub`
- `cmd`

## Services consommés

- `cmd` pour enregistrer les handlers
- `loghub` pour logging

## Services exposés

Aucun.

## Config / NVS

Aucune variable `ConfigStore`.

## Commandes

- `system.ping`
  - réponse: `{"ok":true,"pong":true}`

- `system.reboot`
  - réponse ACK puis `esp_restart()`

- `system.factory_reset`
  - actuellement: réponse ACK puis reboot
  - la purge NVS est notée dans le code comme point d'évolution

## EventBus / DataStore / MQTT

Aucun direct.
