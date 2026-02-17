# LogDispatcherModule (`moduleId: log.dispatcher`)

## Rôle

Consomme les entrées de `LogHub` et les redistribue vers tous les sinks enregistrés.

Type: module passif, mais crée explicitement sa propre task `LogDispatch` (core 1, priority 1, stack 4096).

## Dépendances

- `loghub`

## Services consommés

- `loghub` -> lecture `dequeue`
- `logsinks` -> enumeration des sinks (`count/get`)

## Services exposés

Aucun.

## Config / NVS

Aucune variable `ConfigStore`.

## EventBus / DataStore / MQTT

- aucun EventBus
- aucune écriture DataStore
- aucune publication MQTT

## Détails runtime

Boucle dispatcher:
1. `hub->dequeue(e, portMAX_DELAY)`
2. pour chaque sink enregistré: `sink.write(sink.ctx, e)`

Le dispatch est strictement sériel, sink par sink.
