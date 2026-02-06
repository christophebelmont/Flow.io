# FlowIO Pool Controller — Spécification MQTT (v1)

Version : **1.0**

Ce document décrit la structure des topics MQTT, les payloads JSON, ainsi que les conventions utilisées par le firmware ESP32 de gestion de piscine (FlowIO).

---

## 1) Topic racine

Tous les topics MQTT sont publiés sous :

```
flowio/<deviceId>/
```

Exemple :

```
flowio/ESP32-670D34/
```

> `deviceId` doit être unique et stable (ex : suffixe MAC).

---

## 2) Espaces de noms (namespaces)

Le firmware utilise 4 espaces de noms globaux :

| Namespace | Rôle | Écriture autorisée | Persistant (NVS) | Produit par |
|---|---|---:|---:|---|
| `cfg.*` | Paramètres / configuration | ✅ Oui | ✅ Souvent | UI / MQTT |
| `rt.*` | Valeurs runtime (télémétrie/état) | ❌ Non | ❌ Non | Firmware |
| `cmd.*` | Commandes (actions ponctuelles) | ✅ Oui | ❌ Non | UI / MQTT |
| `evt.*` | Évènements / notifications | ❌ Non | ❌ Non | Firmware |

---

## 3) Règles MQTT (QoS, retained, LWT)

### 3.1 QoS recommandé

| Famille topic | QoS |
|---|---:|
| `rt/*` | 0 (ou 1 si nécessaire) |
| `cfg/*` | 1 |
| `cmd/*` | 1 |
| `evt/*` | 1 |
| `status` | 1 |

### 3.2 retained

| Topic | retained |
|---|---:|
| `status` | ✅ |
| `cfg` | ✅ |
| `rt/*` | ✅ recommandé |
| `cmd/*` | ❌ |
| `evt/*` | ❌ |

> `retained` sur `rt/*` est utile : une UI qui se connecte plus tard voit immédiatement les dernières valeurs.

---

## 4) Présence (LWT / availability)

Topic :

```
flowio/<deviceId>/status
```

Online (retained) :
```json
{"online":true}
```

Offline (LWT retained) :
```json
{"online":false}
```

---

## 5) Topics Runtime (`rt.*`)

Les valeurs runtime sont regroupées par blocs (recommandé, très compatible avec un DataStore + dirty flags).

### 5.1 Liste des topics runtime recommandés

```
flowio/<deviceId>/rt/network
flowio/<deviceId>/rt/sensors
flowio/<deviceId>/rt/controls
flowio/<deviceId>/rt/uptime
flowio/<deviceId>/rt/tanks
flowio/<deviceId>/rt/pid
flowio/<deviceId>/rt/filtration
flowio/<deviceId>/rt/alarms
flowio/<deviceId>/rt/health
```

---

### 5.2 Exemple `rt/network`

Topic :
```
flowio/<deviceId>/rt/network
```

Payload :
```json
{
  "ready": true,
  "ip": "192.168.86.32",
  "rssi": -67
}
```

---

### 5.3 Exemple `rt/sensors`

Topic :
```
flowio/<deviceId>/rt/sensors
```

Payload :
```json
{
  "airTempC": 6.74,
  "waterTempC": 7.62,
  "ph": 7.20,
  "orpMv": -94,
  "pressureBar": 0.00,
  "waterLevelOk": true
}
```

---

### 5.4 Exemple `rt/controls` (pompes, relais, modes)

Topic :
```
flowio/<deviceId>/rt/controls
```

Payload :
```json
{
  "filtration": true,
  "phPump": false,
  "chlPump": false,
  "fillPump": false,
  "swg": false,
  "robot": false,
  "relay0_lights": false,
  "relay1": false,
  "winterMode": false
}
```

---

### 5.5 Exemple `rt/uptime` (compteurs, statuts)

Topic :
```
flowio/<deviceId>/rt/uptime
```

Payload :
```json
{
  "filtrationPumpSec": 0,
  "phPumpSec": 0,
  "chlPumpSec": 0,
  "swgPumpMin": 0,
  "fillPumpMin": 0,

  "filtrationUptimeStatus": "OK",
  "phUptimeStatus": "OK",
  "chlUptimeStatus": "OK",
  "swgUptimeStatus": "OK",
  "fillPumpUptimeStatus": "OK"
}
```

---

### 5.6 Exemple `rt/tanks`

Topic :
```
flowio/<deviceId>/rt/tanks
```

Payload :
```json
{
  "phTankLevelPct": 100.0,
  "chlTankLevelPct": 100.0
}
```

---

### 5.7 Exemple `rt/filtration` (runtime)

Topic :
```
flowio/<deviceId>/rt/filtration
```

Payload :
```json
{
  "startHour": 14,
  "stopHour": 16,
  "durationH": 2
}
```

---

### 5.8 Exemple `rt/pid`

Topic :
```
flowio/<deviceId>/rt/pid
```

Payload :
```json
{
  "ph": {"auto": true, "setpoint": 7.2, "output": 0.0},
  "orp": {"auto": true, "setpoint": 650, "output": 0.0}
}
```

---

### 5.9 Exemple `rt/alarms`

Topic :
```
flowio/<deviceId>/rt/alarms
```

Payload :
```json
{
  "mask": 0,
  "active": []
}
```

---

## 6) Topics Configuration (`cfg.*`)

### 6.1 Snapshot global de configuration

Topic :
```
flowio/<deviceId>/cfg
```

Payload (exemple) :
```json
{
  "mqtt": {"publishPeriodS": 5},

  "filtration": {
    "mode": "auto",
    "startMin": 840,
    "endMax": 960
  },

  "pid": {
    "ph": {"auto": true, "setpoint": 7.2, "windowMin": 5},
    "orp": {"auto": true, "setpoint": 650, "windowMin": 5}
  },

  "limits": {
    "phPumpMaxMin": 30,
    "chlPumpMaxMin": 30,
    "fillPumpMinMin": 5,
    "fillPumpMaxMin": 30,
    "swgMaxMin": 1440
  },

  "thresholds": {
    "psiLow": 0.4,
    "psiHigh": 1.8
  },

  "swg": {
    "mode": "auto",
    "securityTempC": 15.0,
    "startupDelayS": 60
  }
}
```

> Convention : `startMin` / `endMax` sont en **minutes depuis minuit**.

---

## 7) Commandes (`cmd.*`)

### 7.1 Commandes d’action (maintenance)

Topic :
```
flowio/<deviceId>/cmd/action
```

Payloads exemples :
```json
{"name":"reboot"}
{"name":"clearErrors"}
{"name":"resetPhProbeCalibration"}
{"name":"resetOrpProbeCalibration"}
{"name":"resetPressureProbeCalibration"}
{"name":"fillPhTank"}
{"name":"fillChlTank"}
```

---

### 7.2 Commande contrôle d’un actionneur

Topic :
```
flowio/<deviceId>/cmd/control
```

Payloads exemples :
```json
{"name":"filtration","value":true}
{"name":"phPump","value":false}
{"name":"chlPump","value":true}
{"name":"relay0_lights","value":true}
{"name":"robot","value":false}
{"name":"winterMode","value":true}
```

---

### 7.3 Modes (filtration/SWG/PID)

Topic :
```
flowio/<deviceId>/cmd/mode
```

Payloads exemples :
```json
{"filtrationMode":"auto"}
{"filtrationMode":"manual"}
{"swgMode":"auto"}
{"swgMode":"manual"}
{"phAuto":true}
{"orpAuto":false}
```

---

## 8) Écriture de configuration (`cfg/set`) + accusé réception

### 8.1 Appliquer une config (partielle ou complète)

Topic :
```
flowio/<deviceId>/cfg/set
```

Payload exemple :
```json
{"mqtt":{"publishPeriodS":10}}
```

Exemple partiel :
```json
{"pid":{"ph":{"setpoint":7.15}}}
```

### 8.2 Ack

Topic :
```
flowio/<deviceId>/cfg/ack
```

Succès :
```json
{"ok":true}
```

Erreur :
```json
{"ok":false,"err":"invalid_field:pid.ph.setpoint"}
```

---

## 9) Évènements (`evt.*`)

Topic :
```
flowio/<deviceId>/evt
```

Payloads exemples :
```json
{"type":"ntpSynced","ts":1700000000}
{"type":"alarmRaised","alarm":"PSI_LOW","ts":1700000000}
{"type":"alarmCleared","alarm":"PSI_LOW","ts":1700000123}
{"type":"doseStarted","pump":"ph","ts":1700001000}
{"type":"doseStopped","pump":"chl","reason":"limit_reached","ts":1700001020}
```

---

## 10) Mapping projet piscine (valeurs et champs)

### 10.1 Controls → `rt/controls`
- `chlPump` (bool)
- `filtration` (bool)
- `phPump` (bool)
- `fillPump` (bool)
- `relay0_lights` (bool)
- `relay1` (bool)
- `robot` (bool)
- `swg` (bool)
- `winterMode` (bool)

### 10.2 Sensors → `rt/sensors`
- `airTempC` (float °C)
- `waterTempC` (float °C)
- `ph` (float)
- `orpMv` (int/float mV)
- `pressureBar` (float bar)
- `waterLevelOk` (bool)

### 10.3 Tanks → `rt/tanks`
- `phTankLevelPct` (float %)
- `chlTankLevelPct` (float %)

### 10.4 Uptime/status → `rt/uptime`
- `chlPumpSec` (uint32 sec)
- `phPumpSec` (uint32 sec)
- `filtrationPumpSec` (uint32 sec)
- `fillPumpMin` (uint32 min)
- `swgPumpMin` (uint32 min)
- `...Status` (string/enum)

### 10.5 Config → `cfg/*` (extraits)
- `cfg.mqtt.publishPeriodS` (uint16)
- `cfg.filtration.mode` ("auto"/"manual")
- `cfg.filtration.startMin` (uint16)
- `cfg.filtration.endMax` (uint16)
- `cfg.pid.ph.auto` (bool)
- `cfg.pid.ph.setpoint` (float)
- `cfg.pid.ph.windowMin` (uint16)
- `cfg.pid.orp.auto` (bool)
- `cfg.pid.orp.setpoint` (int/float)
- `cfg.pid.orp.windowMin` (uint16)
- `cfg.limits.phPumpMaxMin` (uint16)
- `cfg.limits.chlPumpMaxMin` (uint16)
- `cfg.limits.fillPumpMinMin` (uint16)
- `cfg.limits.fillPumpMaxMin` (uint16)
- `cfg.limits.swgMaxMin` (uint16)
- `cfg.thresholds.psiLow` (float)
- `cfg.thresholds.psiHigh` (float)
- `cfg.swg.mode` ("auto"/"manual")
- `cfg.swg.securityTempC` (float)
- `cfg.swg.startupDelayS` (uint16)

---

## 11) Recommandations firmware (ESP32)

- éviter `String` / allocations heap dans les publications MQTT
- publication runtime recommandée : event-driven (DataStore dirty flags)
- throttling runtime : min 1 publication / seconde / bloc
- traiter `cmd/*` dans un module dédié (CommandModule)
- répondre systématiquement via `cfg/ack` et/ou `cmd/ack` (si implémenté)
