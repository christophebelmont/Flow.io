# LogSerialSinkModule (`moduleId: log.sink.serial`)

## Rôle

Sink de logs vers `Serial` avec:
- niveau colorisé
- timestamp local si l'heure est synchronisée
- fallback uptime sinon

Type: module passif.

## Dépendances

- `loghub`

## Services consommés

- `logsinks` pour enregistrer son sink
- `time` (optionnel) pour formatter l'heure locale

## Services exposés

Aucun.

## Config / NVS

Aucune variable `ConfigStore`.

## EventBus / DataStore / MQTT

Aucun.

## Format de sortie

`[timestamp][level][tag] message`

Priorité timestamp:
1. `TimeService.formatLocalTime()` si `isSynced==true`
2. heure système (`time(nullptr)`) si valide
3. uptime formaté `HH:MM:SS.mmm`
