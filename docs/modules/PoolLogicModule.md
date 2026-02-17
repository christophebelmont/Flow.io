# PoolLogicModule (`moduleId: poollogic`)

## Rôle

Orchestrateur métier de la piscine:
- calcule et maintient la fenêtre quotidienne de filtration à partir de la température d'eau
- pilote les équipements via `PoolDeviceService` (filtration, robot, électrolyse, remplissage)
- applique les règles automatiques (modes, seuils, délais, sécurités)
- expose des commandes métier et des entités Home Assistant
- surveille la pression via `AlarmService`

Type: module actif.

## Dépendances

- `loghub`
- `eventbus`
- `time` (`TimeSchedulerService`)
- `io`
- `pooldev`
- `ha`
- `cmd`
- `alarms`

## Affinité / cadence

- core: 1
- task: `poollogic`
- loop: 200ms

## Services exposés

Aucun service public.

## Services consommés

- `time.scheduler` (`TimeSchedulerService`)
- `io` (`IOServiceV2`)
- `pooldev` (`PoolDeviceService`)
- `cmd` (`CommandService`)
- `alarms` (`AlarmService`)
- `ha` (`HAService`)
- `eventbus` (`EventBus`)

## Config / NVS

Branche: `ConfigBranchId::PoolLogic` -> module JSON `poollogic`, persisté NVS via `NvsKeys::PoolLogic::*`.

Paramètres persistants:
- activation/modes: `enabled`, `auto_mode`, `winter_mode`, `ph_auto_mode`, `orp_auto_mode`, `electrolys_mode`, `electro_run_md`
- calcul fenêtre: `wat_temp_lo_th`, `wat_temp_setpt`, `filtr_start_min`, `filtr_stop_max`
- résultats calculés: `filtr_start_clc`, `filtr_stop_clc`
- bindings IO: `orp_io_id`, `psi_io_id`, `wat_temp_io_id`, `air_temp_io_id`, `pool_lvl_io_id`
- seuils: `psi_low_th`, `psi_high_th`, `winter_start_t`, `freeze_hold_t`, `secure_elec_t`, `orp_setpoint`
- délais: `psi_start_dly_s`, `delay_pids_min`, `dly_electro_min`, `robot_delay_min`, `robot_dur_min`, `fill_min_on_s`
- slots équipements: `filtration_slot`, `swg_slot`, `robot_slot`, `filling_slot`

## Commandes

Enregistrées via `CommandService`:
- `poollogic.filtration.write`
  - args: `{"value":true|false}`
  - force `auto_mode=false` puis écrit le désiré sur le slot de filtration (`pooldev.writeDesired`)
- `poollogic.filtration.recalc`
  - met en file une recomputation de fenêtre (traitée par la loop)
- `poollogic.auto_mode.set`
  - args: `{"value":true|false}`
  - persiste et applique `auto_mode`

Erreurs: format `ErrorCode` (missing args/value, not ready, disabled, interlock, io, etc.).

## EventBus

Abonnements:
- `EventId::SchedulerEventTriggered`
  - `POOLLOGIC_EVENT_DAILY_RECALC` + `Trigger` -> queue recalc
  - `TIME_EVENT_SYS_DAY_START` + `Trigger` -> reset quotidien interne (`cleaningDone_`)
  - `POOLLOGIC_EVENT_FILTRATION_WINDOW` + `Start/Stop` -> état `filtrationWindowActive_`

Publications:
- aucune publication EventBus directe dans ce module

## Scheduler (TimeSchedulerService)

Slots manipulés:
- slot `3` (`POOLLOGIC_EVENT_DAILY_RECALC`)
  - rappel quotidien au pivot (15:00) pour recalcul
- slot `4` (`POOLLOGIC_EVENT_FILTRATION_WINDOW`)
  - fenêtre active start/stop déterminée par `computeFiltrationWindowDeterministic()`
  - `replayStartOnBoot=true` pour reconstruire l'état après reboot

## DataStore / EventStore

- `PoolLogicModule` n'écrit pas directement dans `DataStore`
- il lit l'état capteurs/actionneurs via services (`IOServiceV2`, `PoolDeviceService`, `AlarmService`)
- pas d'`EventStore` persistant (architecture globale: EventBus volatile + ConfigStore persistant)

## Algorithme de contrôle

Entrées runtime:
- analogiques: ORP, PSI, température eau, température air
- digital: niveau bassin
- états actionneurs: lecture `pooldev.readActualOn`

Logique principale:
- filtration:
  - en auto: suit fenêtre scheduler, mode hiver, freeze-hold, et coupe sur erreur PSI
  - en manuel (`auto_mode=false`): conserve pilotage manuel
- robot:
  - démarrage après délai filtration, arrêt à durée max, un seul cycle/jour (`cleaningDone_`)
- électrolyse:
  - nécessite filtration active + sécurité température + délai
  - mode ORP optionnel (`electro_run_md`) avec hystérésis implicite (seuil 100% et 90%)
- remplissage:
  - démarre si niveau bas
  - respecte `fill_min_on_s` avant autoriser arrêt sur retour niveau OK

Alarmes PSI:
- `AlarmId::PoolPsiLow` (latched, délai ON 2s, OFF 1s, répétition 60s)
- `AlarmId::PoolPsiHigh` (latched, critique, délai ON 0ms, OFF 1s, répétition 60s)

## Home Assistant

Entités enregistrées:
- switches config:
  - `pool_auto_mode` (topic source `cfg/poollogic`)
  - `pool_winter_mode` (topic source `cfg/poollogic`)
- sensors:
  - `calculated_filtration_start`
  - `calculated_filtration_stop`
- button:
  - `filtration_recalc` -> commande MQTT `{"cmd":"poollogic.filtration.recalc"}`

## MQTT (indirect)

Le module ne publie pas MQTT directement, mais alimente:
- `cfg/poollogic` via `ConfigStore` (variables persistantes + valeurs calculées)
- commande via `cmd` (`poollogic.*`) relayée par `MQTTModule`

