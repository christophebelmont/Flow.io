# Guide de décision: Core Services vs EventBus vs DataStore

Ce guide décrit quand créer un service dans `src/Core/Services/`, et quand préférer `EventBus` ou `DataStore`.

## 1) Règle simple

Utiliser un **Core Service** quand un module expose une **capacité appelable** (API) dont d'autres modules ont besoin.

Utiliser **EventBus** quand on diffuse une **notification asynchrone** à plusieurs consommateurs.

Utiliser **DataStore** pour l'**état runtime partagé** (dernière valeur connue), avec dirty flags/snapshots.

## 2) Quand utiliser un Core Service

Créer un service Core si les 4 conditions ci-dessous sont vraies:

1. Il y a un producteur unique (ou clairement propriétaire) de la fonctionnalité.
2. Les consommateurs ont besoin d'appels directs (synchrones), pas seulement d'une notification.
3. Le contrat est stable et réutilisable (plus d'un usage ponctuel).
4. L'API n'est pas un simple "état courant" qui peut vivre dans `DataStore`.

Exemples existants:
- `CommandService` pour enregistrer/exécuter des commandes.
- `MqttService` pour publier/formatter des topics.
- `TimeService` pour exposer l'état NTP et l'heure.

## 3) Quand NE PAS créer un Core Service

Ne pas créer de service Core si:

1. Le besoin est une notification "fire-and-forget": utiliser `EventBus`.
2. Le besoin est de lire la dernière valeur d'un état: utiliser `DataStore`.
3. Le contrat est local à un module et n'a pas de consommateur réel.
4. Le service dupliquerait une capacité déjà disponible via un service existant.

Exemple:
- Les valeurs capteurs/actionneurs I/O ne doivent pas devenir un service "get/set global".
  Elles doivent rester dans `DataStore` + publications runtime MQTT.

## 4) Exemple: créer un Core Service (producer/consumer)

### 4.1 Interface (Core)

```cpp
// src/Core/Services/IExample.h
struct ExampleService {
    bool (*doThing)(void* ctx, int value);
    void* ctx;
};
```

Puis l'ajouter à `src/Core/Services/Services.h`.

### 4.2 Producteur (module)

```cpp
class ExampleModule : public Module {
    static bool doThing_(void* ctx, int value);
    ExampleService svc_{ doThing_, this };

    void init(ConfigStore&, ServiceRegistry& services) override {
        services.add("example", &svc_);
    }
};
```

### 4.3 Consommateur

```cpp
void ConsumerModule::init(ConfigStore&, ServiceRegistry& services) {
    exampleSvc_ = services.get<ExampleService>("example");
}
```

Toujours:
- déclarer la dépendance module (`dependency()`),
- vérifier `nullptr` côté consommateur.

## 5) Exemple appliqué IOModule

Le service I/O LED mask est désormais centralisé côté Core:
- interface: `src/Core/Services/IIO.h` (`IOLedMaskService`)
- id service: `io_leds`
- producteur: `IOModule` (enregistre quand PCF8574 est disponible)

Ce choix est correct car:
- c'est une capacité actionnable (set/turnOn/turnOff/getMask),
- potentiellement utilisée par plusieurs consommateurs,
- indépendante du modèle d'état runtime.

## 6) Checklist avant d'ajouter un service Core

1. Qui est le propriétaire unique de la capacité?
2. Ai-je besoin d'appels directs plutôt que d'événements?
3. Est-ce de l'état (DataStore) ou une action/API (Service)?
4. Le contrat sera-t-il réutilisé dans au moins 2 contextes?
5. Puis-je nommer clairement l'id de service et gérer son absence?

Si la réponse est "non" à 2 ou plus de ces questions, privilégier `EventBus` ou `DataStore`.
