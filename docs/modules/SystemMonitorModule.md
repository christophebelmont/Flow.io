# SystemMonitorModule

## Description generale

`SystemMonitorModule` produit des metriques de sante (heap, stack tasks, etat wifi, uptime) et des logs periodiques pour le diagnostic runtime.

## Dependances module

- `moduleId`: `sysmon`
- Dependances declarees: `loghub`

## Services proposes

- Aucun service expose.

## Services consommes

- `wifi` (`WifiService`) pour etat/connectivite/IP
- `config` (`ConfigStoreService`) (pointeur recupere pour extension, non exploite en profondeur ici)
- `loghub` (`LogHubService`) pour emission de logs

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore` propre.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture directe `DataStore`.
