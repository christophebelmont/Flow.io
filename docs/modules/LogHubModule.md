# LogHubModule

## Description generale

`LogHubModule` centralise la collecte des logs applicatifs et maintient le registre des sinks de sortie. C'est la brique de base du pipeline de logs.

## Dependances module

- `moduleId`: `loghub`
- Dependances declarees: aucune

## Services proposes

- `loghub` -> `LogHubService` (enqueue asynchrone des `LogEntry`)
- `logsinks` -> `LogSinkRegistryService` (enregistrement/lecture des sinks)

## Services consommes

- Aucun service consomme.

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore`.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture `DataStore`.
