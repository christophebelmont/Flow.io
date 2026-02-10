# PoolDeviceModule

## Description generale

`PoolDeviceModule` est la couche metier de pilotage des equipements piscine. Il encapsule les regles d'interverrouillage, les commandes `pool.write`/`pool.refill`, les compteurs de marche et les volumes injectes.

## Dependances module

- `moduleId`: `pooldev`
- Dependances declarees: `loghub`, `datastore`, `cmd`, `time`, `io`, `mqtt`, `eventbus`
- Remarque: la logique principale consomme surtout `cmd`, `datastore` et `eventbus`; les autres dependances garantissent un ordre d'initialisation coherent du domaine.

## Services proposes

- Aucun service expose.

## Services consommes

- `cmd` (`CommandService`) pour:
  - enregistrer `pool.write`
  - enregistrer `pool.refill`
  - executer `io.write` en interne
- `eventbus` (`EventBusService`) pour consommer `SchedulerEventTriggered` (reset day/week/month)
- `datastore` (`DataStoreService`) pour lire les sorties IO et publier le runtime pool
- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Modules JSON dynamiques: `pdm/pdN` (N selon les devices definis, max 8)
- Cles par device:
  - `enabled`
  - `type`
  - `depends_on_mask`
  - `flow_l_h`
  - `tank_capacity_ml`
  - `tank_initial_ml`

## Valeurs DataStore utilisees

- Ecrit:
  - `pool.devices[idx]` via `setPoolDeviceRuntime`
  - Plage DataKeys: `DATAKEY_POOL_DEVICE_BASE + idx` (`DATAKEY_POOL_DEVICE_BASE` = 80)
  - Dirty flag: `DIRTY_SENSORS`
- Lu:
  - `io.endpoints[idx]` (etat reel des sorties digitales associees)
