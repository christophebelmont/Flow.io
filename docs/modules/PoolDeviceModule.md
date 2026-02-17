# PoolDeviceModule (`moduleId: pooldev`)

## Rôle

Couche domaine actionneurs piscine:
- inventorie les 8 slots `pd0..pd7` et leur mapping I/O
- applique commandes désirées avec interlocks de dépendance
- synchronise état réel I/O et état désiré
- comptabilise runtime (jour/semaine/mois/total) et volumes injectés
- persiste les métriques runtime dans `ConfigStore` (`pdmrt/pdN.metrics_blob`)
- expose snapshots runtime pour publication MQTT

Type: module actif.

## Dépendances

- `loghub`
- `datastore`
- `cmd`
- `time`
- `io`
- `mqtt`
- `eventbus`
- `ha`

## Affinité / cadence

- core: 1
- task: `pooldev`
- loop: 200ms (avec retry init toutes 250ms tant que runtime non prêt)

## Services exposés

- `pooldev` -> `PoolDeviceService`
  - `count()`
  - `meta(slot, outMeta)`
  - `readActualOn(slot, outOn, outTsMs)`
  - `writeDesired(slot, on)`
  - `refillTank(slot, remainingMl)`

Codes retour: `POOLDEV_SVC_OK`, `...UNKNOWN_SLOT`, `...NOT_READY`, `...DISABLED`, `...INTERLOCK`, `...IO`.

## Services consommés

- `io` (`IOServiceV2`) pour lire/écrire sorties digitales
- `datastore` (`DataStoreService`) pour runtime partagé
- `eventbus` (`EventBus`) pour resets périodiques et resynchronisation
- `cmd` (`CommandService`) pour handlers commande
- `ha` (`HAService`) pour discovery capteurs/paramètres
- `config` (`ConfigStore`) via API module

## Config / NVS

Branches config par slot:
- métier: `pdm/pd0` ... `pdm/pd7` (`ConfigBranchId::PoolDevicePd0..Pd7`)
- runtime persistant: `pdmrt/pd0` ... `pdmrt/pd7` (`ConfigBranchId::PoolDeviceRuntimePd0..Pd7`)

Clés persistantes (format):
- `pd%uen` -> `enabled`
- `pd%uty` -> `type`
- `pd%udp` -> `depends_on_mask`
- `pd%uflh` -> `flow_l_h`
- `pd%utc` -> `tank_cap_ml`
- `pd%uti` -> `tank_init_ml`
- `pd%urt` -> `metrics_blob` (runtime persistant)

Format `metrics_blob`:
- `v1,<running_day_ms>,<running_week_ms>,<running_month_ms>,<running_total_ms>,<inj_day_ml>,<inj_week_ml>,<inj_month_ml>,<inj_total_ml>,<tank_ml>,<day_key>,<week_key>,<month_key>`

## Commandes

Enregistrées:
- `pooldevice.write`
- `pool.write` (compat legacy)
  - args: `{"slot":N,"value":true|false|0|1}`
  - applique `writeDesired` avec contrôles interlock/enable/io
- `pool.refill`
  - args: `{"slot":N,"remaining_ml":1234.0}` (`remaining_ml` optionnel)
  - met à jour niveau cuve suivi et force commit runtime/persist

## EventBus

Abonnements:
- `EventId::SchedulerEventTriggered`
  - `TIME_EVENT_SYS_DAY_START` -> reset compteurs jour
  - `TIME_EVENT_SYS_WEEK_START` -> reset semaine
  - `TIME_EVENT_SYS_MONTH_START` -> reset mois
- `EventId::DataChanged`
  - sur `DATAKEY_TIME_READY` à `true` -> demande reconcile de période
- `EventId::ConfigChanged`
  - branche `ConfigBranchId::Time` -> demande reconcile de période (ex: `week_start_mon`)

Publications:
- aucune publication EventBus directe

## DataStore

Écritures via `PoolDeviceRuntime.h`:
- état: `setPoolDeviceRuntimeState(ds, slot, ...)`
  - clé `DataKeys::PoolDeviceStateBase + slot` (`80..87`)
  - dirty flag `DIRTY_ACTUATORS`
- métriques: `setPoolDeviceRuntimeMetrics(ds, slot, ...)`
  - clé `DataKeys::PoolDeviceMetricsBase + slot` (`88..95`)
  - dirty flag `DIRTY_SENSORS`

Données runtime:
- state: `enabled`, `desiredOn`, `actualOn`, `type`, `blockReason`, `tsMs`
- metrics: `runningSecDay/Week/Month/Total`, `injectedMl*`, `tankRemainingMl`, `tsMs`

## Persistance runtime (important)

Objectif:
- conserver les runtimes à travers reboot
- reset uniquement sur changement de période (jour/semaine/mois)

Mécanisme:
- chargement au boot (`onConfigLoaded`) depuis `metrics_blob`
- reconcile de période basé horloge locale + config `time.week_start_mon`
- reset ciblé quand marqueur période (`dayKey/weekKey/monthKey`) change
- persistance conditionnelle:
  - immédiate si arrêt équipement ou reset période ou refill
  - sinon max toutes les `60s` pendant fonctionnement (`RUNTIME_PERSIST_INTERVAL_MS`)

## Scheduler / période

Le module maintient des clés:
- `dayKey = YYYYMMDD`
- `weekKey = date locale du début de semaine`
- `monthKey = YYYYMM`

`currentPeriodKeys_()` n'est valide que si:
- `timeReady == true`
- epoch >= `2021-01-01` (`MIN_VALID_EPOCH_SEC`)

Si l'heure n'est pas prête: reconcile replanifié.

## Contrôle des dépendances (interlocks)

Chaque slot peut dépendre d'autres slots (`dependsOnMask`):
- au démarrage d'un slot, tous les slots dépendance doivent être `actualOn`
- si dépendance tombe, arrêt forcé + `blockReason=interlock`

Blocages possibles:
- `none`
- `disabled`
- `interlock`
- `io_error`

## Snapshots runtime (MQTT indirect)

Le module implémente `IRuntimeSnapshotProvider`:
- `runtimeSnapshotCount()` -> `2 * nb_slots_actifs`
- `runtimeSnapshotSuffix(idx)`:
  - `rt/pdm/state/pdN`
  - `rt/pdm/metrics/pdN`
- `buildRuntimeSnapshot(idx, ...)` -> JSON complet state/metrics

Publication assurée par `MQTTModule` via mux runtime global.

## Home Assistant

Entités créées:
- sensors uptime journalière (chlorine/ph/fill/filtration/chlorine_generator selon slots présents)
  - sources `rt/pdm/metrics/pdN`
- number sliders `flow_l_h` pour `pd0`, `pd1`, `pd2`
  - source `cfg/pdm/pdN`
  - commande `cfg/set` patch JSON

## Initialisation des slots (main)

Les slots `pd0..pd7` sont définis via `defineDevice()` depuis `main.cpp` à partir de `FLOW_POOL_IO_BINDINGS`:
- `pd0` filtration
- `pd1` pH (peristaltique, cuve, dépend filtration)
- `pd2` chlore (peristaltique, cuve, dépend filtration)
- `pd3` robot (dépend filtration)
- `pd4` remplissage
- `pd5` électrolyse (dépend filtration)
- `pd6` lumières
- `pd7` chauffage eau

