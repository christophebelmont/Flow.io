# ConfigStoreModule (`moduleId: config`)

## Rôle

Expose `ConfigStore` en service utilisable par les autres modules (JSON apply/export/list).

Type: module passif.

## Dépendances

- `loghub`

## Services exposés

- `config` -> `ConfigStoreService`
  - `applyJson`
  - `toJson`
  - `toJsonModule`
  - `listModules`

## Services consommés

- `loghub` (log d'initialisation)

## Config / NVS

Ne déclare pas ses propres variables, il expose l'instance globale `ConfigStore`.

## EventBus / DataStore / MQTT

Directement: aucun.

Indirectement via `ConfigStore`:
- un `applyJson` qui modifie une variable persistante publie `EventId::ConfigChanged`
- utilisé par `MQTTModule` pour `cfg/set`

## Usage développeur

Exemple d'application d'un patch:

```cpp
const ConfigStoreService* cfg = services.get<ConfigStoreService>("config");
if (cfg) cfg->applyJson(cfg->ctx, "{\"poollogic\":{\"auto_mode\":true}}");
```
