# ModuleDevGuide

Guide pratique pour ajouter un nouveau module dans Flow.io (ESP32 + FreeRTOS), avec conventions de code et documentation attendue.

## 1) Choisir le type de module

- `Module` (actif): le module a une boucle runtime (`loop`) et sa propre task FreeRTOS.
- `ModulePassive` (passif): le module fait du wiring uniquement (services/config), sans task.

## 2) Creer les fichiers du module

Creer au minimum:

- `src/Modules/<Nom>/<Nom>Module.h`
- `src/Modules/<Nom>/<Nom>Module.cpp`
- Optionnel runtime: `src/Modules/<Nom>/<Nom>ModuleDataModel.h`
- Optionnel helpers runtime: `src/Modules/<Nom>/<Nom>Runtime.h`

## 3) Definir le contrat du module

Dans le header du module:

- definir `moduleId()` stable et explicite
- definir `taskName()` pour les modules actifs
- declarer `dependencyCount()` et `dependency(i)`
- implementer `init(ConfigStore&, ServiceRegistry&)`
- implementer `loop()` (module actif)

## 4) Gestion ConfigStore

- declarer des `ConfigVariable<...>` dans le module
- appeler `cfg.registerVar(...)` dans `init`
- regrouper les cles par `moduleName` coherent (ex: `mqtt`, `io/input/a0`, `pdm/pd0`)
- ne pas acceder directement a NVS hors `ConfigStore`

## 5) Gestion DataStore

Si le module publie du runtime partage:

1. Ajouter une structure dans `<Nom>ModuleDataModel.h`.
2. Ajouter le marker `MODULE_DATA_MODEL`.
3. Creer des helpers set/get dans `<Nom>Runtime.h`.
4. Notifier les changements via `ds.notifyChanged(dataKey, dirtyMask)`.
5. Regenerer les agregats:

```bash
python3 scripts/generate_datamodel.py
```

## 6) Services: quoi exposer

- Exposer un service seulement pour une capacite appelable reutilisable.
- Utiliser `services.add("id", &service)` dans le module producteur.
- Consommer via `services.get<T>("id")` et verifier les `nullptr`.
- Voir aussi `docs/CoreServicesGuidelines.md`.

## 7) EventBus: quoi publier/souscrire

- Publier des evenements pour les transitions asynchrones.
- Garder les payloads compacts et trivially copyable.
- Eviter le traitement lourd dans le callback EventBus.

## 8) Wiring dans `main.cpp`

- instancier le module
- l'ajouter au `ModuleManager`
- pour les modules metier, declarer les definitions runtime (ex: I/O, pool devices)
- verifier l'ordre logique (les dependances reelles restent garanties par `ModuleManager`)

## 9) Documentation module obligatoire

Pour chaque nouveau module, creer `docs/modules/<Nom>Module.md` avec exactement la structure suivante:

1. `Description generale`
2. `Dependances module`
3. `Services proposes`
4. `Services consommes`
5. `Valeurs ConfigStore utilisees`
6. `Valeurs DataStore utilisees`

Cette structure doit rester identique pour tous les modules afin de faciliter la revue et la maintenance.

## 10) Checklist avant merge

- le module compile
- les dependances sont exactes
- les services sont enregistres/consommes correctement
- les cles ConfigStore sont documentees
- les cles/champs DataStore sont documentes
- la documentation module est ajoutee dans `docs/modules/`
- le lien est ajoute dans `README.md`
