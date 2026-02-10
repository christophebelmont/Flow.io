# TimeModule — Synchronisation horaire et Scheduler

Ce document décrit le module `time` (ancien `NTPModule`) et son scheduler intégré.

## 1) Rôle du module

`TimeModule` fournit deux capacités:

- synchroniser l’horloge système (backend actuel: NTP),
- exposer un scheduler statique (16 slots) pour des évènements ponctuels/récurrents.

Le contrat est volontairement générique: un backend futur (RTC, source externe, etc.) pourra remplacer/compléter NTP sans changer l’API publique.

## 2) Services Core exposés

### 2.1 Service `time`

Interface: `src/Core/Services/ITime.h`

- `state(ctx)` → `TimeSyncState`
- `isSynced(ctx)` → bool
- `epoch(ctx)` → epoch secondes
- `formatLocalTime(ctx, out, len)` → format local `YYYY-MM-DD HH:MM:SS`

### 2.2 Service `time.scheduler`

Interface: `src/Core/Services/ITimeScheduler.h`

- `setSlot(ctx, slotDef)`
- `getSlot(ctx, slot, outDef)`
- `clearSlot(ctx, slot)`
- `clearAll(ctx)`
- `usedCount(ctx)`
- `activeMask(ctx)` (`uint16_t`)
- `isActive(ctx, slot)`

## 3) Modèle Scheduler

### 3.1 Capacité et types

- capacité fixe: `TIME_SCHED_MAX_SLOTS = 16`
- un slot peut être:
  - un `Event` (`hasEnd=false`),
  - un `TimeSlot` (`hasEnd=true`, avec start/stop).

Un `TimeSlot` actif est visible via `activeMask` (bit `N` = slot `N` actif).

### 3.2 Champs d’un slot

`TimeSchedulerSlot`:

- `slot` (0..15)
- `eventId` (identifiant métier)
- `enabled`
- `hasEnd`
- `replayStartOnBoot`
- `label` (debug, max 23 chars + `\0`)
- `mode`:
  - `RecurringClock` (heure locale + jours),
  - `OneShotEpoch` (epoch absolu)
- récurrence:
  - `weekdayMask`, `startHour`, `startMinute`, `endHour`, `endMinute`
- one-shot:
  - `startEpochSec`, `endEpochSec`

Constantes utiles:

- jours: `TIME_WEEKDAY_MON..TIME_WEEKDAY_SUN`
- masques: `TIME_WEEKDAY_NONE`, `TIME_WEEKDAY_WORKDAYS`, `TIME_WEEKDAY_WEEKEND`, `TIME_WEEKDAY_ALL`

### 3.3 Slots système réservés

Les 3 premiers slots sont réservés et recréés automatiquement:

- `0` → `TIME_EVENT_SYS_DAY_START`
- `1` → `TIME_EVENT_SYS_WEEK_START`
- `2` → `TIME_EVENT_SYS_MONTH_START`

Propriétés:

- non modifiables via `setSlot`/`clearSlot`,
- `clearAll` les conserve,
- labels par défaut: `sys_day_start`, `sys_week_start`, `sys_month_start`,
- évènement mois: déclenché à 00:00 uniquement si `tm_mday == 1`.

## 4) Config globale (`ConfigStore`)

Module JSON: `time`

- `server1` (`ntp_s1`) — serveur NTP principal
- `server2` (`ntp_s2`) — serveur NTP secondaire
- `tz` (`ntp_tz`) — timezone POSIX
- `enabled` (`ntp_en`) — active la synchro
- `week_start_monday` (`tm_wkmon`) — `true=lundi`, `false=dimanche`

Module JSON: `time/scheduler`

- `slots_blob` (`tm_sched`) — persistance des slots utilisateur

Compatibilité:

- les clés NVS historiques `ntp_*` sont conservées.

## 5) Évènements EventBus

Évènement publié: `EventId::SchedulerEventTriggered`

Payload: `SchedulerEventTriggeredPayload` (`src/Core/EventBus/EventPayloads.h`)

- `slot`
- `edge` (`Trigger`, `Start`, `Stop`)
- `replayed` (1 = replay au boot/résync)
- `eventId`
- `epochSec`
- `activeMask` (`uint16_t`)

Le module logge chaque émission:

- `LOGI("Scheduler event ...")` pour `trigger/start/stop`.

## 6) Lien module métier ↔ scheduler

Le lien se fait par `eventId` (pas par label interne du module).

Flux conseillé:

1. Le module métier réserve ses `eventId` (dans Core ou module métier).
2. Il configure ses slots via `time.scheduler.setSlot`.
3. Il s’abonne à `EventId::SchedulerEventTriggered`.
4. Il filtre sur `eventId` et `edge`.

Cela évite de hardcoder des noms métier dans `TimeModule`.

## 7) Intégration PoolDeviceModule

`PoolDeviceModule` écoute `SchedulerEventTriggered` et exploite les slots système:

- `TIME_EVENT_SYS_DAY_START` → reset compteurs day
- `TIME_EVENT_SYS_WEEK_START` → reset compteurs week
- `TIME_EVENT_SYS_MONTH_START` → reset compteurs month

La remise à zéro est différée via un masque pending, puis appliquée dans la boucle `tickDevices_()` (thread-safe).

## 8) Commandes debug

Commandes enregistrées:

- `time.resync`
- `time.scheduler.info`
- `time.scheduler.get`
- `time.scheduler.set`
- `time.scheduler.clear`
- `time.scheduler.clear_all`

Exemples (`cmd` MQTT args JSON):

```json
{"cmd":"time.scheduler.info","args":{}}
```

```json
{"cmd":"time.scheduler.get","args":{"slot":4}}
```

```json
{"cmd":"time.scheduler.set","args":{
  "slot":4,
  "event_id":1201,
  "label":"filtration_window",
  "mode":"recurring_clock",
  "enabled":true,
  "has_end":true,
  "replay_start_on_boot":true,
  "weekday_mask":127,
  "start_hour":9,
  "start_minute":0,
  "end_hour":17,
  "end_minute":0
}}
```

```json
{"cmd":"time.scheduler.clear","args":{"slot":4}}
```

```json
{"cmd":"time.scheduler.clear_all","args":{}}
```

## 9) Mode test horloge accélérée

Dans `src/Modules/Network/TimeModule/TimeModule.cpp`, activer:

```cpp
// #define TIME_TEST_FAST_CLOCK
```

Puis décommenter la ligne.

Comportement:

- démarrage simulé au `2020-01-01 00:00:00`,
- `5 minutes` réelles = `1 mois` simulé.

Permet de tester rapidement les bascules day/week/month et les scheduler slots.

## 10) Limites actuelles et évolutions

- backend temps alternatif (RTC, etc.) non implémenté,
- pas de mode `catch-up` (rejeu massif d’évènements après longue coupure),
- persistance scheduler en blob compact (pas format JSON lisible),
- mapping `eventId`/usage métier à maintenir dans Core ou les modules métier.

Pour un futur mode catch-up, prévoir:

- borne max d’évènements rejoués au boot,
- mode dégradé “skip to now” configurable.
