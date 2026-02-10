# EventBusModule

## Description generale

`EventBusModule` heberge l'instance centrale `EventBus`, execute la boucle de dispatch et publie l'evenement de demarrage systeme.

## Dependances module

- `moduleId`: `eventbus`
- Dependances declarees: `loghub`

## Services proposes

- `eventbus` -> `EventBusService` (acces au bus d'evenements)

## Services consommes

- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore`.

## Valeurs DataStore utilisees

- Aucune lecture/ecriture directe `DataStore`.
