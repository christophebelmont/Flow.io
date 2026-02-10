# Flow.io - Gestion et regulation de piscine sur ESP32

Flow.io est un firmware ESP32 conçu pour piloter une piscine de façon fiable et industrialisable: mesures en continu (pH, ORP, pression, temperatures), commande d'equipements (pompes, electrolyseur, eclairage, chauffage, remplissage), supervision runtime et remontee des metriques vers MQTT/Home Assistant. L'objectif produit est simple: transformer une installation piscine en systeme connecte, pilotable et observable en temps reel, avec une architecture robuste et evolutive.

## Architecture modulaire ESP32

Le programme est entierement modulaire:

- chaque fonctionnalite est implementee dans un module autonome (`Module` ou `ModulePassive`)
- les dependances entre modules sont explicites (`dependencyCount`, `dependency`)
- les services inter-modules sont types et enregistres dans `ServiceRegistry`
- l'etat runtime partage est centralise dans `DataStore` (avec dirty flags et evenements)
- la configuration persistante est centralisee dans `ConfigStore` (NVS + import/export JSON)
- les modules actifs tournent dans leur propre task FreeRTOS

Ce design est adapte a l'ESP32: couplage faible, evolutions maitrisees et integration progressive de nouveaux capteurs/actionneurs.

## Structure globale du projet

```text
src/
  Core/
    Module*.h/.cpp                 # socle module manager + services + runtime
    Services/                      # interfaces de services (IWifi, IMqtt, ITime, ...)
    DataStore/                     # etat runtime + notifications EventBus
    EventBus/                      # bus d'evenements inter-modules
  Modules/
    Logs/                          # hub de logs + dispatcher + sink serie
    Stores/                        # ConfigStoreModule + DataStoreModule
    System/                        # commandes systeme + monitoring
    Network/                       # Wifi, Time, MQTT, Home Assistant
    IOModule/                      # capteurs/actionneurs, bus, drivers, endpoints
    PoolDeviceModule/              # couche metier equipements piscine
    EventBusModule/                # service eventbus
    CommandModule/                 # registre de commandes
docs/
  modules/                         # documentation detaillee module par module
  CoreServicesGuidelines.md        # regles de design des services core
  ModuleDevGuide.md                # guide pour creer un nouveau module
```

## Modules disponibles

- Logs: `loghub`, `log.dispatcher`, `log.sink.serial`
- Core runtime: `eventbus`, `config`, `datastore`, `cmd`
- Systeme: `system`, `sysmon`
- Reseau: `wifi`, `time`, `mqtt`, `ha`
- Metier piscine: `io`, `pooldev`

## Documentation complete

### Documentation module par module (meme structure)

- [LogHubModule](docs/modules/LogHubModule.md)
- [LogDispatcherModule](docs/modules/LogDispatcherModule.md)
- [LogSerialSinkModule](docs/modules/LogSerialSinkModule.md)
- [EventBusModule](docs/modules/EventBusModule.md)
- [CommandModule](docs/modules/CommandModule.md)
- [ConfigStoreModule](docs/modules/ConfigStoreModule.md)
- [DataStoreModule](docs/modules/DataStoreModule.md)
- [SystemModule](docs/modules/SystemModule.md)
- [SystemMonitorModule](docs/modules/SystemMonitorModule.md)
- [WifiModule](docs/modules/WifiModule.md)
- [TimeModule](docs/modules/TimeModule.md)
- [MQTTModule](docs/modules/MQTTModule.md)
- [HAModule](docs/modules/HAModule.md)
- [IOModule](docs/modules/IOModule.md)
- [PoolDeviceModule](docs/modules/PoolDeviceModule.md)

### Guides transverses

- [CoreServicesGuidelines](docs/CoreServicesGuidelines.md)
- [ModuleDevGuide](docs/ModuleDevGuide.md)
- [DevGuide (historique)](docs/DevGuide.md)

## Build

```bash
pio run
```
