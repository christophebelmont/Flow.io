# IOModule — Architecture et configuration

Ce document décrit le module `io` actuellement implémenté.

Il remplace l’ancien découpage `Sensors/Actuators` par une architecture unifiée:
- bus (`I2C`, `OneWire`)
- drivers (`ADS1115`, `DS18B20`, `PCF8574`, GPIO)
- endpoints (`analog input`, `digital actuator`, `mask endpoint`)
- registry + scheduler

## 1) Comportement global

- Un seul module runtime: `IOModule` (`moduleId = "io"`).
- Poll ADS asynchrone via scheduler (`ads_poll_ms`, défaut `125 ms`).
- DS18B20 asynchrone via scheduler (`ds_poll_ms`, défaut `2000 ms`).
- Pipeline analogique par endpoint:
  1. lecture driver
  2. validation `min/max`
  3. `RunningMedian(11)` + moyenne centrale (`5`)
  4. calibration `y = c0*x + c1`
  5. arrondi à la précision
  6. callback métier (`onValueChanged`) défini dans `main.cpp`

Important: le module `io` ne contient plus de logique métier `ph/orp/psi/...`.

## 2) ConfigStore du module `io`

Toutes les clés ci-dessous sont dans le module JSON `"io"`.

### 2.1 Paramètres généraux

| Clé JSON | Type | Défaut | Description |
|---|---:|---:|---|
| `enabled` | bool | `true` | Active/désactive le module IO |
| `i2c_sda` | int | `21` | Pin SDA |
| `i2c_scl` | int | `22` | Pin SCL |
| `ads_poll_ms` | int | `125` | Période scheduler ADS |
| `ds_poll_ms` | int | `2000` | Période scheduler DS18B20 |
| `ads_internal_addr` | uint8 | `0x48` | Adresse ADS interne |
| `ads_external_addr` | uint8 | `0x49` | Adresse ADS externe |
| `ads_gain` | int | `0` | Code PGA ADS1115 |
| `ads_rate` | int | `1` | Data rate ADS (0..7) |
| `pcf_enabled` | bool | `false` | Active le driver PCF8574 |
| `pcf_address` | uint8 | `0x20` | Adresse I2C PCF8574 |
| `pcf_mask_default` | uint8 | `0` | Masque initial écrit au boot |

### 2.2 Valeurs possibles `ads_gain`

`ads_gain` est un **code** (pas un multiplicateur):

| `ads_gain` | Macro ADS1X15 | Full scale | LSB théorique (ADS1115) |
|---:|---|---|---|
| `0` | `ADS1X15_GAIN_6144MV` | ±6.144 V | 0.1875 mV/bit |
| `1` | `ADS1X15_GAIN_4096MV` | ±4.096 V | 0.1250 mV/bit |
| `2` | `ADS1X15_GAIN_2048MV` | ±2.048 V | 0.0625 mV/bit |
| `4` | `ADS1X15_GAIN_1024MV` | ±1.024 V | 0.03125 mV/bit |
| `8` | `ADS1X15_GAIN_0512MV` | ±0.512 V | 0.015625 mV/bit |
| `16` | `ADS1X15_GAIN_0256MV` | ±0.256 V | 0.0078125 mV/bit |

### 2.3 Valeurs possibles `source` (endpoints analogiques)

Pour chaque slot analogique:

- `0` = `IO_SRC_ADS_INTERNAL_SINGLE`
- `1` = `IO_SRC_ADS_EXTERNAL_DIFF`
- `2` = `IO_SRC_DS18_WATER`
- `3` = `IO_SRC_DS18_AIR`

`channel` selon source:
- ADS single: `0..3`
- ADS diff: `0` => diff `0-1`, `1` => diff `2-3`
- DS18B20: ignoré

### 2.4 Configuration des slots analogiques

Le module expose 5 slots configurables via ConfigStore:
- `a0_*`, `a1_*`, `a2_*`, `a3_*`, `a4_*`

Chaque slot a les clés:
- `aN_source`
- `aN_channel`
- `aN_c0`
- `aN_c1`
- `aN_prec`
- `aN_min`
- `aN_max`

`N` est l’index de slot (0..4).

Le lien entre un slot et un nom métier (`ph`, `orp`, etc.) est défini uniquement dans `main.cpp` par l’ordre des `defineAnalogInput(...)`.

## 3) Endpoint PCF8574 “mask”

Quand `pcf_enabled=true`, le module crée l’endpoint:
- `status_leds_mask`

Type endpoint:
- `IO_EP_DIGITAL_ACTUATOR`
- valeur `int32` utilisée sur 8 bits (`mask`)

Sémantique:
- bit `0..7` = état sortie PCF correspondant
- écriture globale du masque possible
- helpers `turnOn(bit)` / `turnOff(bit)` disponibles via service

Service enregistré (si PCF actif):
- id service: `io_leds`
- interface: `IOLedMaskService`
  - `setMask(mask)`
  - `turnOn(bit)`
  - `turnOff(bit)`
  - `getMask()`

## 4) Runtime DataStore

Runtime IO stocké dans:

```cpp
struct IORuntimeData {
    IOEndpointRuntime endpoints[IO_MAX_ENDPOINTS];
};
```

Helpers disponibles (`IORuntime.h`):
- `setIoEndpointFloat(ds, idx, value, tsMs, dirtyMask)`
- `ioEndpointFloat(ds, idx, out)`

Dans l’implémentation actuelle, `main.cpp` mappe:
- `idx=0` -> pH
- `idx=1` -> ORP
- `idx=2` -> PSI
- `idx=3` -> water temp
- `idx=4` -> air temp

## 5) Exemple de patch config JSON

Topic:
- `flowio/<deviceId>/cfg/set`

Exemple:

```json
{
  "io": {
    "enabled": true,
    "i2c_sda": 21,
    "i2c_scl": 22,
    "ads_internal_addr": 72,
    "ads_external_addr": 73,
    "ads_gain": 0,
    "ads_rate": 1,
    "ads_poll_ms": 125,
    "ds_poll_ms": 2000,

    "a0_source": 0,
    "a0_channel": 0,
    "a0_c0": 1.0,
    "a0_c1": 0.0,
    "a0_prec": 1,
    "a0_min": -32768,
    "a0_max": 32767,

    "pcf_enabled": true,
    "pcf_address": 32,
    "pcf_mask_default": 0
  }
}
```

Notes:
- `72` = `0x48`, `73` = `0x49`, `32` = `0x20`.
- Les clés non fournies conservent leur valeur actuelle.
