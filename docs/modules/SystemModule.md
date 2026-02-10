# SystemModule

## Description generale

`SystemModule` enregistre les commandes systeme de base (ping, reboot, factory reset) exposees via le bus de commande.

## Dependances module

- `moduleId`: `system`
- Dependances declarees: `loghub`, `cmd`

## Services proposes

- Aucun service expose.

## Services consommes

- `cmd` (`CommandService`) pour enregistrer:
  - `system.ping`
  - `system.reboot`
  - `system.factory_reset`
- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore`.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture `DataStore`.
