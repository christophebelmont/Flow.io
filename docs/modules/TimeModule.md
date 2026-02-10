# TimeModule

## Description generale

`TimeModule` synchronise l'horloge (NTP), maintient un scheduler de slots temporels (16 slots) et publie des evenements de planification metier.

## Dependances module

- `moduleId`: `time`
- Dependances declarees: `loghub`, `datastore`, `cmd`, `eventbus`

## Services proposes

- `time` -> `TimeService`
- `time.scheduler` -> `TimeSchedulerService`

## Services consommes

- `eventbus` (`EventBusService`) pour:
  - consommer `DataChanged`/`ConfigChanged`
  - publier `SchedulerEventTriggered`
- `cmd` (`CommandService`) pour enregistrer:
  - `time.resync`, `ntp.resync`
  - `time.scheduler.info|get|set|clear|clear_all`
- `datastore` (`DataStoreService`) pour publier l'etat de synchro
- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Module JSON: `time`
  - `server1`
  - `server2`
  - `tz`
  - `enabled`
  - `week_start_monday`
- Module JSON: `time/scheduler`
  - `slots_blob`

## Valeurs DataStore utilisees

- Ecrit:
  - `time.timeReady` (DataKey `DATAKEY_TIME_READY` = 3, dirty `DIRTY_TIME`)
- Lu:
  - `wifi.ready` pour piloter la transition `WaitingNetwork -> Syncing`
