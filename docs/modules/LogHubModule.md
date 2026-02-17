# LogHubModule (`moduleId: loghub`)

## Rôle

Point d'entrée central du logging asynchrone.
- héberge `LogHub` (queue de logs)
- héberge le registre de sinks (`LogSinkRegistry`)
- expose les services de log pour tous les modules

Type: module passif (`ModulePassive`, pas de task).

## Dépendances

Aucune dépendance déclarée.

## Services exposés

- `loghub` -> `LogHubService`
- `logsinks` -> `LogSinkRegistryService`

Le module appelle aussi `Log::setHub(&hubSvc)` pour activer les helpers globaux `Log::info/debug/...`.

## Config / NVS

Aucune variable `ConfigStore`.

## EventBus / DataStore / MQTT

- aucun abonnement EventBus
- aucune publication EventBus
- aucune écriture DataStore
- aucune publication MQTT directe

## Points d'extension

- ajouter un nouveau sink via `logsinks.add(...)`
- ajuster la profondeur de queue via `Limits::LogQueueLen`
