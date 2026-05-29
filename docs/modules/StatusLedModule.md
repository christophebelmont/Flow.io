# StatusLedModule (`moduleId: status_led`)

## Rôle

Pilote la LED RGB WS2812 intégrée de l'ESP32-S3 (GPIO 48) comme indicateur de statut visuel.
La LED reflète en temps réel l'état du système selon une machine à priorités décroissantes.

Profil cible : **FlowIOS3** uniquement.  
Type : module actif (tâche FreeRTOS dédiée).

---

## États et indications visuelles

Les états sont évalués à chaque tour de boucle (~10 ms). Le premier état dont la condition est vraie est appliqué.

| Priorité | État | Couleur | Animation | Condition |
|---|---|---|---|---|
| 1 | **OTA en cours** | Cyan | Blink rapide | `FirmwareUpdateService::isBusy() == true` |
| 2 | **Alarme critique** | Rouge | Blink rapide | `AlarmSeverity >= Critical` et alarme active |
| 3 | **Alarme autre** | Orange | Blink lent | `AlarmSeverity >= Warning` et alarme active |
| 4 | **Portail WiFi** | Bleu | Blink lent | `NetworkAccessService::mode() == AccessPoint` |
| 5 | **Pas de WiFi** | Jaune | Blink rapide | `wifi.ready == false` (après première connexion) |
| 6 | **WiFi OK / pas MQTT** | Jaune | Blink lent | `wifi.ready == true && mqtt.mqttReady == false` |
| 7 | **Nominal** | Vert | Fixe | Tout connecté, aucune alarme active |
| — | **Boot** *(état initial)* | Blanc | Breathing | Avant que WiFi se connecte pour la première fois |

### Correspondance sévérités d'alarme

| `AlarmSeverity` | État LED déclenché |
|---|---|
| `Critical` (3) | AlarmCritical → rouge blink rapide |
| `Alarm` (2) | AlarmOther → orange blink lent |
| `Warning` (1) | AlarmOther → orange blink lent |
| `Info` (0) | Aucun effet |

---

## Animations

| Animation | Période | Description |
|---|---|---|
| **Fixe** | — | LED allumée en continu |
| **Blink rapide** | 200 ms | 100 ms ON / 100 ms OFF (5 Hz) |
| **Blink lent** | 1000 ms | 500 ms ON / 500 ms OFF (1 Hz) |
| **Breathing** | 2000 ms | Intensité en triangle linéaire (0 → max → 0) |

Luminosité maximale : **25/255** (~10 %) — visible sans être éblouissant.  
Constante `kBr` dans `StatusLedModule.h` pour ajuster.

---

## Dépendances

Dépendances déclarées (tri topologique garanti) :
- `DataStore` — lecture de `wifi.ready` et `mqtt.mqttReady`
- `Alarm` — lecture de `activeCount()` et `highestSeverity()`

Services résolus en `onStart()` (optionnels, null-safe) :
- `FirmwareUpdate` (`ServiceId::FirmwareUpdate`) — détection OTA via `isBusy()`
- `NetworkAccess` (`ServiceId::NetworkAccess`) — détection portail AP via `mode()`

---

## Intégration

### Fichiers

| Fichier | Rôle |
|---|---|
| `src/Modules/System/StatusLedModule/StatusLedModule.h` | Déclaration du module |
| `src/Modules/System/StatusLedModule/StatusLedModule.cpp` | Implémentation |
| `src/Profiles/FlowIOS3/FlowIOS3Profile.h` | Instance `statusLedModule{}` |
| `src/Profiles/FlowIOS3/FlowIOS3Bootstrap.cpp` | `moduleManager.add()` en premier |

### Ordre d'enregistrement

Le module est enregistré **en premier** dans `registerModules()` afin que la LED s'allume dès le démarrage du boot, avant même l'initialisation du log ou du WiFi.

---

## Affinité / cadence

- Core : 1
- Stack : 1536 octets
- Priorité FreeRTOS : 1 (basse)
- Cadence effective : ~10 ms (délai `Limits::Core::Timing::LoopDelayMs`)

---

## Services consommés

| Service | ID | Usage |
|---|---|---|
| `DataStoreService` | `ServiceId::DataStore` | Lecture `wifi.ready`, `mqtt.mqttReady` |
| `AlarmService` | `ServiceId::Alarm` | `activeCount()`, `highestSeverity()` |
| `FirmwareUpdateService` | `ServiceId::FirmwareUpdate` | `isBusy()` — optionnel |
| `NetworkAccessService` | `ServiceId::NetworkAccess` | `mode()` — optionnel |

## Services exposés

Aucun.

## Config / NVS

Aucune. Le module est entièrement stateless (pas de `ConfigVariable`).

## EventBus / DataStore / MQTT

Aucun abonnement, aucune publication. Le module lit directement le `DataStore` à chaque loop.

---

## Matériel

- **GPIO** : 48 (LED WS2812 intégrée ESP32-S3)
- **Driver** : `neopixelWrite(pin, r, g, b)` — fourni par `esp32-hal-rgb-led` (inclus via `<Arduino.h>`)
- **Profil concerné** : `FlowIOS3`, `FlowIOS3Wokwi`

> **Note Wokwi** : le simulateur Wokwi supporte la LED GPIO 48 de l'ESP32-S3. Les animations seront visibles en simulation.
