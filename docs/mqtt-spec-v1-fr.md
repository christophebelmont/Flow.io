# FlowIO — Spécification MQTT (v1)

Version : **1.1** (document unifié)

Ce document décrit la structure des topics MQTT, les payloads JSON, et les conventions utilisées par le firmware.

---

## 1) Topic racine

```
flowio/<deviceId>/
```

Exemple :
```
flowio/ESP32-670D34/
```

---

## 2) Espaces de noms

| Namespace | Rôle | Écriture autorisée | Persistant (NVS) | Produit par |
|---|---|---:|---:|---|
| `cfg` | Configuration | ✅ Oui | ✅ Souvent | UI / MQTT |
| `rt/*` | Runtime | ❌ Non | ❌ Non | Firmware |
| `cmd` | Commandes | ✅ Oui | ❌ Non | UI / MQTT |
| `evt` | Évènements | ❌ Non | ❌ Non | Firmware |

---

## 3) Topics principaux

### Commandes
```
flowio/<deviceId>/cmd
flowio/<deviceId>/ack
```

### Statut
```
flowio/<deviceId>/status
```

### Configuration
```
flowio/<deviceId>/cfg/set
flowio/<deviceId>/cfg/ack
flowio/<deviceId>/cfg/<module>
```

### Runtime (snapshots)
```
flowio/<deviceId>/rt/sensors/state
flowio/<deviceId>/rt/network/state
flowio/<deviceId>/rt/system/state
```

---

## 4) Payloads runtime (exemples)

### `rt/sensors/state`
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

### `rt/network/state`
```json
{
  "ready": true,
  "ip": "192.168.86.28",
  "rssi": -60,
  "mqtt": true,
  "ts": 12345678
}
```

### `rt/system/state`
```json
{
  "upt_ms": 12345678,
  "heap": {"free": 180000, "min": 160000, "largest": 90000, "frag": 15},
  "ts": 12345678
}
```

---

## 5) Périodicité par défaut

- `rt/sensors/state` : 15 s
- `rt/network/state` : 60 s
- `rt/system/state` : 60 s
