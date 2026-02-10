# CommandModule

## Description generale

`CommandModule` centralise l'enregistrement et l'execution des commandes (`system.*`, `io.write`, `time.*`, etc.) via un registre commun.

## Dependances module

- `moduleId`: `cmd`
- Dependances declarees: `loghub`

## Services proposes

- `cmd` -> `CommandService` (`registerHandler`, `execute`)

## Services consommes

- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore`.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture `DataStore`.
