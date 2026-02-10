# LogDispatcherModule

## Description generale

`LogDispatcherModule` depile les logs du hub puis les distribue a tous les sinks enregistres (`Serial`, etc.) dans une task dediee.

## Dependances module

- `moduleId`: `log.dispatcher`
- Dependances declarees: `loghub`

## Services proposes

- Aucun service expose.

## Services consommes

- `loghub` (`LogHubService`) pour acceder a la file de logs
- `logsinks` (`LogSinkRegistryService`) pour iterer et ecrire vers les sinks

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore`.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture `DataStore`.
