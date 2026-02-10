# Core Services Guidelines

Ce guide formalise quand creer un service Core dans `src/Core/Services/` et quand preferer `EventBus` ou `DataStore`.

## 1) Regle de decision rapide

- Utiliser un **service Core** pour exposer une **capacite appelable** (API synchrone) a plusieurs modules.
- Utiliser **EventBus** pour diffuser une **notification asynchrone** (fire-and-forget).
- Utiliser **DataStore** pour partager un **etat runtime** (derniere valeur connue + dirty flags).

## 2) Criteres pour creer un service Core

Creer un service seulement si les conditions suivantes sont reunies:

1. Un proprietaire clair de la capacite existe (module producteur unique).
2. Les consommateurs ont besoin d'un appel direct (pas uniquement d'un evenement).
3. Le contrat est stable (reutilisable, pas ponctuel).
4. Le besoin n'est pas un simple etat qui devrait vivre dans `DataStore`.

## 3) Criteres pour NE PAS creer un service Core

Ne pas creer de service si:

1. Le besoin est un trigger asynchrone multi-consommateurs (`EventBus`).
2. Le besoin est la lecture d'un etat (`DataStore`).
3. Le contrat n'a qu'un seul usage local et temporaire.
4. Un service equivalent existe deja.

## 4) Inventaire des services Core du projet

- `eventbus` -> `EventBusService` (owner: `EventBusModule`)
- `config` -> `ConfigStoreService` (owner: `ConfigStoreModule`)
- `datastore` -> `DataStoreService` (owner: `DataStoreModule`)
- `cmd` -> `CommandService` (owner: `CommandModule`)
- `wifi` -> `WifiService` (owner: `WifiModule`)
- `mqtt` -> `MqttService` (owner: `MQTTModule`)
- `time` -> `TimeService` (owner: `TimeModule`)
- `time.scheduler` -> `TimeSchedulerService` (owner: `TimeModule`)
- `io_leds` -> `IOLedMaskService` (owner: `IOModule`, seulement si PCF8574 actif)
- `loghub` -> `LogHubService` (owner: `LogHubModule`)
- `logsinks` -> `LogSinkRegistryService` (owner: `LogHubModule`)

## 5) Regles d'implementation

- Ajouter l'interface dans `src/Core/Services/`.
- Ajouter l'include dans `src/Core/Services/Services.h`.
- Enregistrer le service dans le module producteur avec `services.add("service-id", &svc)`.
- Declarer les dependances des consommateurs via `dependency()`.
- Cote consommateur, verifier les pointeurs retournes par `services.get<...>()`.

## 6) Frontiere Service vs DataStore dans Flow.io

- Les commandes d'action (`io.write`, `pool.write`) restent des commandes metier; l'etat resultant est publie dans `DataStore`.
- Les etats `wifi.ready`, `mqtt.ready`, `time.ready`, `io.*`, `pool.*` appartiennent au runtime (`DataStore`).
- Les orchestrations asynchrones (ex: scheduler -> reset compteurs pool) passent par `EventBus`.

## 7) Checklist avant d'ajouter un nouveau service

1. Qui est proprietaire unique de la capacite?
2. Pourquoi un appel direct est necessaire?
3. Pourquoi `EventBus` est insuffisant?
4. Pourquoi `DataStore` est insuffisant?
5. Quels modules consomment reellement ce service aujourd'hui?
6. Quel `service-id` stable sera utilise?

Si 2 reponses ou plus sont faibles ou ambiguës, preferer `EventBus` et/ou `DataStore`.
