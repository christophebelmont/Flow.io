# LogSerialSinkModule

## Description generale

`LogSerialSinkModule` enregistre un sink d'affichage sur port serie, avec couleur par niveau et timestamp (heure locale si dispo, sinon uptime).

## Dependances module

- `moduleId`: `log.sink.serial`
- Dependances declarees: `loghub`

## Services proposes

- Aucun service expose.

## Services consommes

- `logsinks` (`LogSinkRegistryService`) pour ajouter le sink serie

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore`.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture `DataStore`.
