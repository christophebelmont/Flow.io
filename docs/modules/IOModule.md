# IOModule (`moduleId: io`)

## Rôle

Couche I/O unifiée:
- inventaire des endpoints (analogiques, entrées/sorties digitales)
- drivers/bus (GPIO, ADS1115, DS18B20, PCF8574)
- scheduler d'acquisition
- exposition `IOServiceV2`
- snapshots runtime pour MQTT
- enregistrement entités HA (capteurs analogiques + switches équipements)

Type: module actif.

## Dépendances

- `loghub`
- `datastore`
- `mqtt`
- `ha`

## Affinité / cadence

- core: 1
- task: `io`
- loop: 10ms (tick scheduler interne)

## Services exposés

- `io` -> `IOServiceV2`
  - découverte: `count`, `idAt`, `meta`
  - lecture: `readDigital`, `readAnalog`
  - écriture: `writeDigital`
  - cycle: `tick`, `lastCycle`

## Services consommés

- `datastore`
- `ha`
- `loghub`

## Config / NVS

Branches:
- `Io` + `IoDebug`
- `IoInputA0..A5`
- `IoOutputD0..D7`

Paramètres principaux:
- bus/scheduling: `i2c_sda`, `i2c_scl`, `ads_poll_ms`, `ds_poll_ms`, `digital_poll_ms`
- ADS: adresse interne/externe, gain, rate
- PCF: enable/address/mask/active_low
- traces: `trace_enabled`, `trace_period_ms`
- calibration/labels/precision par entrée analogique `a0..a5`
- mapping sorties digitales `d0..d7` (pin, active_high, momentary, pulse)

## DataStore

Écritures via `IORuntime.h`:
- `setIoEndpointFloat(...)` (dirty sensors)
- `setIoEndpointBool(...)` (dirty actuators pour sorties)
- `setIoEndpointInt(...)`

Clés:
- base `DataKeys::IoBase` + index endpoint

## Snapshots runtime

`IRuntimeSnapshotProvider`:
- suffixes publiés:
  - `rt/io/input/aN`
  - `rt/io/input/iN`
  - `rt/io/output/dN`
- payload endpoint standard:
  - `id`, `name`, `value`, `ts`

## HA intégration

- capteurs analogiques enregistrés (`ORP`, `pH`, `PSI`, `Spare`, températures)
- switches HA pour sorties pool (`cmd` MQTT avec payload JSON de commande)
- refresh HA déclenché si changement de précision analogique

## Détails runtime

- jobs scheduler internes:
  - `ads_fast`
  - `ds_slow`
  - `din_poll`
- support sorties impulsionnelles (`momentary/pulseMs`)
- cycle info (`IoCycleInfo`) maintenu pour deltas
