# FlowIO — Spécification MQTT (v1.1)

Version : **1.1**

Cette version conserve **un topic de commande** et **un ack unique**, avec publication des snapshots runtime (sensors/network/system).

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

`deviceId` doit être unique et stable (ex : suffixe MAC).

---

## 2) Espaces de noms

| Namespace | Rôle | Écriture autorisée | Persistant (NVS) | Produit par |
|---|---|---:|---:|---|
| `cfg` | Configuration | ✅ Oui | ✅ Souvent | UI / MQTT |
| `rt/*` | Runtime (télémétrie/état) | ❌ Non | ❌ Non | Firmware |
| `cmd` | Commandes | ✅ Oui | ❌ Non | UI / MQTT |
| `evt` | Évènements | ❌ Non | ❌ Non | Firmware |

---

## 3) QoS & retained (recommandations)

| Famille topic | QoS | retained |
|---|---:|---:|
| `status` | 1 | ✅ |
| `cfg/*` | 1 | ✅ |
| `rt/*` | 0 (ou 1) | ✅ recommandé |
| `cmd` | 0/1 | ❌ |
| `ack` | 0/1 | ❌ |
| `evt` | 1 | ❌ |

---

## 4) Présence

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

## 5) Commandes

Topic :
```
flowio/<deviceId>/cmd
```
Payload :
```json
{"cmd":"system.ping","args":{}}
```

Ack :
```
flowio/<deviceId>/ack
```
Payload :
```json
{"ok":true,"cmd":"system.ping","reply":{"ok":true,"pong":true}}
```

---

## 6) Configuration

### 6.1 Snapshot par blocs

Topics :
```
flowio/<deviceId>/cfg/<module>
```

Exemples :
```
flowio/<deviceId>/cfg/mqtt
flowio/<deviceId>/cfg/io
```

### 6.2 Appliquer une config

Topic :
```
flowio/<deviceId>/cfg/set
```
Payload :
```json
{"mqtt":{"baseTopic":"flowio2"}}
```

Ack :
```
flowio/<deviceId>/cfg/ack
```

---

## 7) Runtime (snapshots)

### 7.1 Sensors

Topic :
```
flowio/<deviceId>/rt/sensors/state
```
Payload :
```json
{
  "ph": 7.12,
  "orp": 732.0,
  "psi": 1.8,
  "waterTemp": 26.4,
  "airTemp": 24.1,
  "ts": 12345678
}
```

### 7.2 Network

Topic :
```
flowio/<deviceId>/rt/network/state
```
Payload :
```json
{
  "ready": true,
  "ip": "192.168.86.28",
  "rssi": -60,
  "mqtt": true,
  "ts": 12345678
}
```

### 7.3 System

Topic :
```
flowio/<deviceId>/rt/system/state
```
Payload :
```json
{
  "upt_ms": 12345678,
  "heap": {"free": 180000, "min": 160000, "largest": 90000, "frag": 15},
  "ts": 12345678
}
```

---

## 8) Périodicité par défaut

- `rt/sensors/state` : 15 s
- `rt/network/state` : 60 s
- `rt/system/state` : 60 s

Ces périodes sont configurables via `cfg/sensors` et `cfg/mqtt`.
