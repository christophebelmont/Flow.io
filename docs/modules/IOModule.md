# IOModule

## Description generale

`IOModule` est la couche d'abstraction I/O du projet: bus (I2C, OneWire), drivers (ADS1115, DS18B20, GPIO, PCF8574), endpoints capteurs/actionneurs, scheduler de polling et commande runtime `io.write`.

## Dependances module

- `moduleId`: `io`
- Dependances declarees: `loghub`, `datastore`, `cmd`, `mqtt`
- Note: la dependance `mqtt` est surtout structurelle (ordre d'initialisation), le module ne consomme pas directement `MqttService`.

## Services proposes

- `io_leds` -> `IOLedMaskService` (uniquement si PCF8574 actif)
  - `setMask`
  - `turnOn`
  - `turnOff`
  - `getMask`

## Services consommes

- `cmd` (`CommandService`) pour enregistrer `io.write`
- `datastore` (`DataStoreService`) pour publier les changements runtime d'endpoints
- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Module JSON: `io`
  - `enabled`
  - `i2c_sda`, `i2c_scl`
  - `ads_poll_ms`, `ds_poll_ms`
  - `ads_internal_addr`, `ads_external_addr`
  - `ads_gain`, `ads_rate`
  - `pcf_enabled`, `pcf_address`, `pcf_mask_default`, `pcf_active_low`
- Modules JSON analogiques: `io/input/a0` ... `io/input/a4`
  - `aN_name`, `aN_source`, `aN_channel`, `aN_c0`, `aN_c1`, `aN_prec`, `aN_min`, `aN_max`
- Modules JSON sorties: `io/output/d0` ... `io/output/d7`
  - `dN_name`, `dN_pin`, `dN_active_high`, `dN_initial_on`, `dN_momentary`, `dN_pulse_ms`

## Valeurs DataStore utilisees

- Ecrit:
  - `io.endpoints[idx]` via helpers `setIoEndpoint*`
  - Plage DataKeys: `DATAKEY_IO_BASE + idx` (`DATAKEY_IO_BASE` = 40)
  - Dirty flags:
    - `DIRTY_ACTUATORS` pour les ecritures de commandes (`io.write`)
    - `DIRTY_SENSORS` quand le runtime met a jour des mesures via callbacks integrateur
- Lu:
  - lecture du runtime `io.endpoints[]` pour certains snapshots/etats
- Note d'integration:
  - les mesures analogiques sont remontees au `DataStore` via callback `onValueChanged` defini lors du wiring (ex: `main.cpp`).
