# flow.io sur Waveshare ESP32-S3

flow.io est une plateforme autonome de gestion de piscine. Son matériel de référence est désormais le module industriel **Waveshare ESP32-S3-POE-ETH-8DI-8RO**, piloté par l'environnement PlatformIO `Waveshare-ESP32-S3`.

<p align="center">
  <img src="docs/pictures/waveshare-esp32-s3-poe-eth-8di-8ro.png" alt="Module Waveshare ESP32-S3-POE-ETH-8DI-8RO utilisé par flow.io" width="520">
</p>

<p align="center">
  <a href="https://www.waveshare.com/esp32-s3-eth-8di-8ro.htm">Waveshare ESP32-S3-POE-ETH-8DI-8RO</a> — 8 entrées digitales isolées, 8 relais, Ethernet W5500, Wi-Fi/BLE et RS485.
</p>

Le profil Waveshare réunit sur un même ESP32-S3 la logique piscine, les entrées/sorties, Ethernet et Wi-Fi, le provisioning, l'interface web, MQTT, Home Assistant, les mises à jour et l'interface HMI. Les anciens profils `FlowIO` et `Supervisor` restent présents dans le dépôt pour les installations historiques.

Pour démarrer:

```sh
~/.platformio/penv/bin/pio run -e Waveshare-ESP32-S3
~/.platformio/penv/bin/pio run -e Waveshare-ESP32-S3 -t upload
```

- [Mise en service du profil Waveshare](docs/integration/mise-en-service.md)
- [Cartographie complète des binding ports, IO slots et domain slots](docs/core/waveshare-io-map.md)
- [Documentation technique](docs/README.md)

## Pourquoi flow.io

Sans orchestration continue, on observe vite:
- dérive pH / ORP
- filtration mal dimensionnée par rapport à la température
- surconsommation de produits et d'énergie
- usure prématurée des pompes et actionneurs
- gestion complexe de l'hivernage

flow.io apporte un pilotage cohérent de bout en bout.

![PoolMaster Ecosystem](docs/pictures/PoolMaster%20Ecosystem.png)

## Matériel de référence

Le profil `Waveshare-ESP32-S3` exploite notamment:

- les 8 entrées digitales isolées de la carte, exposées par défaut comme `i00` à `i07`;
- les 8 relais pilotés par le TCA9554, exposés comme `d00` à `d07`;
- l'Ethernet 10/100 via W5500, avec repli Wi-Fi et provisioning local;
- l'horloge temps réel PCF85063ATL, le buzzer et la LED RGB;
- un bus I2C d'extension pour les convertisseurs, capteurs et expanders du profil;
- deux bus 1-Wire pour les sondes de température d'eau et d'air;
- un écran local ST7789 optionnel, câblé par le profil de production.

Le modèle d'E/S sépare volontairement trois niveaux:

| Niveau | Rôle | Exemple |
|---|---|---|
| `domain_slot` | besoin métier piscine | `ActuatorFiltrationPump` |
| `io_slot` | endpoint logique stable | `d00` |
| `binding_port` | ressource physique sélectionnée | `300` / `EXIO1` |

La chaîne complète est donc `domain_slot -> io_slot -> binding_port`. Elle permet de conserver une logique métier stable tout en réaffectant un capteur ou un actionneur à une autre ressource physique. La [cartographie Waveshare](docs/core/waveshare-io-map.md) inventorie tous les ports, slots et bindings par défaut.

## Surveillance et contrôle en continu

flow.io mesure l'état du bassin et pilote les équipements en continu pour maintenir l'eau stable, adapter la filtration et sécuriser les traitements.

Modes de désinfection supportés:
- `Chlore/Brome`: régulation PID temporelle sur sonde ORP, avec injection par pompe péristaltique, consigne redox, délai de stabilisation après démarrage filtration, sécurité pression et contrôle du niveau de cuve
- `Electrolyse`: pilotage d'un électrolyseur au sel, soit en suivi de consigne ORP avec hystérésis, soit sur plages fixes pendant la filtration, avec température minimale de sécurité et temporisation de démarrage
- `Oxygène actif liquide`: dosage volumétrique sans asservissement ORP, calculé à partir du volume du bassin, de la dose produit hebdomadaire, du facteur de charge, de la compensation température optionnelle, de l'heure principale de dosage et d'un fractionnement en 1, 2 ou 3 injections par semaine

Régulation automatique de température:
- consigne de chauffage avec hystérésis et relais chauffage dédié
- protocole de chauffage assisté qui lance d'abord la filtration pour obtenir une mesure fiable de température d'eau, puis décide de maintenir pompe et chauffage actifs
- cycles de sondage périodiques lorsque la pompe est arrêtée, avec arrêt automatique une fois la consigne atteinte
- blocage de sécurité si la pression ou la mesure de température ne sont pas cohérentes

Mesures effectuées:
- température de l'eau et de l'air
- pression de pompe
- pH
- ORP / redox
- niveau du bassin
- niveaux de cuves pH et désinfection
- compteur d'eau ou métriques de remplissage
- états, temps de marche, volumes injectés et historiques d'exploitation des équipements

Actionneurs supportés:
- pompe de filtration
- pompe doseuse pH, compatible pH- ou pH+
- pompe doseuse chlore/brome ou oxygène actif liquide
- électrolyseur au sel
- pompe robot
- pompe ou électrovanne de remplissage
- chauffage ou pompe à chaleur
- éclairage et relais auxiliaires

## Interface locale tactile

L'interface locale tactile offre une vue synthétique des mesures, états et commandes principales pour l'exploitation quotidienne au bord du bassin.

![Nextion TouchScreen HMI2](docs/pictures/Nextion5-2.jpeg)

## Automatisation utile au quotidien

- calcul automatique de la fenêtre de filtration selon la température d'eau
- priorisation et interlock des actionneurs pour une sécurité totale
- gestion des plannings (jour/semaine/mois) persistante
- modes d'exploitation (auto, manuel, protection gel)
- supervision alarmes (pression, états critiques)

## Principe de régulation PID (pH / ORP)

flow.io implémente une régulation PID temporelle pour les pompes péristaltiques pH et ORP:
- calcul PID périodique (par défaut toutes les `30 s`)
- conversion de la sortie en durée d'activation `output_on_ms` bornée dans une fenêtre fixe (`window_ms`, typiquement `1 h`)
- commande ON/OFF dans la fenêtre: la pompe est active en début de fenêtre pendant `output_on_ms`

Si les conditions de sécurité ne sont pas réunies (filtration arrêtée, mode hiver, capteur indisponible, défaut pression, etc.), la sortie est remise à `0` et la pompe est coupée.

Détail complet de l'algorithme, des conditions d'activation et des topics runtime dans la documentation module:
- [PoolLogicModule](docs/modules/PoolLogicModule.md)

## Intégration et exploitation

- publication MQTT structurée (`cfg/*`, `rt/*`, `cmd`, `ack`)
- auto-discovery Home Assistant pour le contrôle sur Internet et les statistiques à long terme
- gestion via application mobile entièrement paramétrable (Home Assistant)
- intégration possible avec Jeedom/Node-RED/InfluxDB/Grafana via MQTT
- architecture modulaire robuste (FreeRTOS + services Core + EventBus + DataStore + ConfigStore/NVS)
- Mises en jour OTA en Wi-Fi

Résultat: une eau plus stable, une maintenance plus prévisible et une meilleure maîtrise des coûts d'exploitation.

![Grafana](docs/pictures/Grafana.png)

## Documentation développeur

La documentation complète (architecture, services Core, flux EventBus/DataStore/MQTT, et fiche détaillée par module) est disponible ici:

- [Documentation complète](docs/README.md)
- [Protocole flow.io <-> Supervisor (I2C cfg/status)](docs/core/flow-supervisor-i2c-protocol.md)
- [Quality Gates Modules (notes + description des 10 points)](docs/core/module-quality-gates.md)
