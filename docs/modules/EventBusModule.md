# EventBusModule (`moduleId: eventbus`)

## Rôle

Hôte du bus d'événements interne `EventBus`.
- enregistre le service `eventbus`
- publie `SystemStarted` au démarrage
- exécute le dispatch des événements

Type: module actif.

## Dépendances

- `loghub`

## Affinité / cadence

- core: 1
- task name: `EventBus`
- stack: 4096
- priority: 1
- loop: `dispatch(8)` puis `delay(5ms)`

## Services exposés

- `eventbus` -> `EventBusService`

## Config / NVS

Aucune variable `ConfigStore`.

## EventBus

Publication:
- `EventId::SystemStarted` (payload nul) dans `init()`

Abonnements:
- aucun (c'est le producteur/dispatcher du bus)

## DataStore / MQTT

Aucun accès direct.
