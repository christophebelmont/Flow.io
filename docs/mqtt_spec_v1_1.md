# FlowIO Pool Controller — Spécification MQTT (v1.1)

Version : **1.1**

Cette version conserve **un unique topic de commande** et **un unique ack**, tout en ajoutant `status` et `cfg/*`.

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

## 2) Espaces de noms (namespaces)

| Namespace | Rôle | Écriture autorisée | Persistant (NVS) | Produit par |
|---|---|---:|---:|---|
| `cfg` | Paramètres / configuration | ✅ Oui | ✅ Souvent | UI / MQTT |
| `rt/*` | Valeurs runtime (télémétrie/état) | ❌ Non | ❌ Non | Firmware |
| `cmd` | Commandes (actions ponctuelles) | ✅ Oui | ❌ Non | UI / MQTT |
| `evt` | Évènements / notifications | ❌ Non | ❌ Non | Firmware |

---

## 3) Règles MQTT (QoS, retained, LWT)

### 3.1 QoS recommandé

| Famille topic | QoS |
|---|---:|
| `rt/*` | 0 (ou 1 si nécessaire) |
| `cfg` | 1 |
| `cfg/ack` | 1 |
| `cfg/set` | 1 |
| `cmd` | 0 ou 1 |
| `ack` | 0 ou 1 |
| `evt` | 1 |
| `status` | 1 |

### 3.2 retained

| Topic | retained |
|---|---:|
| `status` | ✅ |
| `cfg` | ✅ |
| `rt/*` | ✅ recommandé |
| `cmd` | ❌ |
| `ack` | ❌ |
| `evt` | ❌ |
| `cfg/ack` | ❌ |
| `cfg/set` | ❌ |

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

## 5) Commandes (`cmd`) — topic unique

Topic :

```
flowio/<deviceId>/cmd
```

Payload attendu :

```json
{"cmd":"system.ping","args":{}}
```

`cmd` est le nom de commande enregistré dans `CommandService`.
Chaque module déclare ses commandes localement.

### Ack unique

```
flowio/<deviceId>/ack
```

Payload exemple :

```json
{"ok":true,"cmd":"system.ping","reply":{"ok":true,"pong":true}}
```

---

## 6) Configuration (`cfg`)

### 6.1 Snapshot par blocs (recommandé)

Topics :
```
flowio/<deviceId>/cfg/mqtt
flowio/<deviceId>/cfg/filtration
flowio/<deviceId>/cfg/pid
flowio/<deviceId>/cfg/limits
flowio/<deviceId>/cfg/thresholds
flowio/<deviceId>/cfg/swg
```

Payload : JSON de la section correspondante (retained)

### 6.2 Appliquer une config (partielle ou complète)

Topic :
```
flowio/<deviceId>/cfg/set
```

Payload exemple :
```json
{"mqtt":{"baseTopic":"flowio2"}}
```

Exemple partiel :
```json
{"pid":{"ph":{"setpoint":7.15}}}
```

Le parser est tolérant aux espaces.

### 6.3 Ack config

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
{"ok":false}
```

---

## 7) Runtime (`rt/*`) — inchangé

Liste de topics recommandés :
- `rt/network`
- `rt/sensors`
- `rt/controls`
- `rt/uptime`
- `rt/tanks`
- `rt/pid`
- `rt/filtration`
- `rt/alarms`
- `rt/health`

---

## 8) Évènements (`evt`)

Topic :
```
flowio/<deviceId>/evt
```

Payloads exemples :
```json
{"type":"ntpSynced","ts":1700000000}
{"type":"alarmRaised","alarm":"PSI_LOW","ts":1700000000}
```

---

## 9) Recommandations firmware

- éviter `String` / allocations heap dans les publications MQTT
- publication runtime recommandée : event-driven (DataStore dirty flags)
- throttling runtime : min 1 publication / seconde / bloc
- traiter `cmd` dans un module dédié (`CommandModule`)
- répondre systématiquement via `ack` et/ou `cfg/ack`
