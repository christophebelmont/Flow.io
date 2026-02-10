# ConfigStoreModule

## Description generale

`ConfigStoreModule` expose l'acces au `ConfigStore` (patch JSON, export global/module, liste des blocs). Il fournit la facade de configuration pour MQTT, HA et autres modules.

## Dependances module

- `moduleId`: `config`
- Dependances declarees: `loghub`

## Services proposes

- `config` -> `ConfigStoreService`
  - `applyJson`
  - `toJson`
  - `toJsonModule`
  - `listModules`

## Services consommes

- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Ce module expose le store global; il ne declare pas ses propres cles.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture `DataStore`.
