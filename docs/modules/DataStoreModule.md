# DataStoreModule

## Description generale

`DataStoreModule` instancie le `DataStore` runtime global et le met a disposition des autres modules.

## Dependances module

- `moduleId`: `datastore`
- Dependances declarees: `eventbus`

## Services proposes

- `datastore` -> `DataStoreService` (pointeur vers l'instance `DataStore`)

## Services consommes

- `eventbus` (`EventBusService`) pour injecter le bus dans le `DataStore` et permettre les notifications (`DataChanged`, `DataSnapshotAvailable`)

## Valeurs ConfigStore utilisees

- Aucune cle `ConfigStore`.

## Valeurs DataStore utilisees

- Ce module heberge l'objet `DataStore` global.
- Il ne manipule pas de cle metier en propre.
