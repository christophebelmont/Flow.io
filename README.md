# Flow.IO

Flow.IO transforme une installation piscine classique en système industriel connecté: mesure continue de la qualité d'eau, pilotage intelligent des équipements, supervision temps réel et intégration domotique.

## Pourquoi un système de gestion qualité de l'eau piscine

Une piscine n'est pas seulement un volume d'eau: c'est un équilibre physico-chimique fragile qui doit rester stable dans le temps.

Sans pilotage structuré, les dérives sont fréquentes:
- désinfection irrégulière (ORP/chlore)
- pH hors plage de confort/efficacité
- filtration sur- ou sous-dimensionnée selon la température et l'usage
- surconsommation de produits et d'énergie
- usure prématurée des équipements

Flow.IO adresse ces points avec une approche orientée fiabilité:
- acquisition continue des capteurs (pH, ORP, pression, températures, niveau)
- contrôle coordonné des actionneurs (pompes, électrolyse, robot, remplissage, etc.)
- scénarios automatiques (fenêtre de filtration, modes, interlocks)
- télémétrie MQTT et auto-discovery Home Assistant
- architecture modulaire robuste (services typés, EventBus, DataStore, ConfigStore/NVS)

Résultat: eau plus stable, exploitation plus simple, coûts mieux maîtrisés, et meilleure traçabilité opérationnelle.

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
