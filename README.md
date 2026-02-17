# Flow.IO

Flow.IO est une plateforme de pilotage piscine connectée orientée fiabilité: elle automatise la qualité d'eau, réduit les opérations manuelles, et donne une supervision claire des équipements en local comme à distance.

![Home Automation Integration](docs/pictures/Grafana%20and%20App.png)

## Pourquoi Flow.IO

Une piscine est un système physico-chimique vivant. Sans orchestration continue, on observe vite:
- dérive pH / ORP
- filtration mal dimensionnée par rapport à la température
- surconsommation de produits et d'énergie
- usure prématurée des pompes et actionneurs

Flow.IO apporte un pilotage cohérent de bout en bout.

![Flow.IO Hardware](docs/pictures/PoolMasterBox_pf.jpg)

## Mesure et contrôle en continu

Mesures suivies en continu:
- température d'eau et d'air
- pression de filtration
- pH
- ORP
- niveau bassin
- métriques de fonctionnement des équipements (temps de marche, volumes injectés estimés, niveau cuves estimé)

Actionneurs pilotés:
- pompe de filtration
- pompes péristaltiques pH / chlore liquide
- électrolyse (SWG)
- pompe robot
- pompe de remplissage
- relais auxiliaires (ex: éclairage, chauffage, équipements externes)

## Automatisation utile au quotidien

- calcul automatique de la fenêtre de filtration selon la température d'eau
- priorisation sécurité/actionneurs avec interlocks de dépendance
- gestion temporelle (jour/semaine/mois) persistante, y compris après reboot
- modes d'exploitation (auto, manuel, hiver, protection gel)
- supervision alarmes (pression, états critiques)

## Intégration et exploitation

- publication MQTT structurée (`cfg/*`, `rt/*`, `cmd`, `ack`)
- auto-discovery Home Assistant
- intégration possible avec Jeedom/Node-RED/InfluxDB/Grafana via MQTT
- architecture modulaire robuste (FreeRTOS + services Core + EventBus + DataStore + ConfigStore/NVS)

![Phone Interface](docs/pictures/Phone%20Interface.jpg)

Résultat: une eau plus stable, une maintenance plus prévisible et une meilleure maîtrise des coûts d'exploitation.

![Grafana](docs/pictures/Grafana.png)

## Documentation développeur

La documentation complète (architecture, services Core, flux EventBus/DataStore/MQTT, et fiche détaillée par module) est disponible ici:

- [Documentation complète](docs/README.md)

## Référence rapide

- Firmware: ESP32 + FreeRTOS
- Configuration persistante: `ConfigStore` sur NVS (`Preferences`)
- Runtime partagé: `DataStore`
- Bus d'événements: `EventBus`
- Transport externe: MQTT
- Intégration domotique: Home Assistant discovery
