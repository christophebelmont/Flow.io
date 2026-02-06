# Guide Développeur — Modules, DataStore, EventBus

Ce document donne des exemples concrets pour :
- créer un module
- enregistrer services, commandes, variables de config
- ajouter des variables runtime au DataStore
- publier des événements sur l’EventBus avec un payload dédié

---

## 1) Créer un nouveau module

### 1.1 Fichiers recommandés

Crée un dossier module, par exemple :
- `src/Modules/ExampleModule/ExampleModule.h`
- `src/Modules/ExampleModule/ExampleModule.cpp`

Ajoute ensuite le module dans `src/main.cpp` (comme les autres).

### 1.2 Squelette de module (service + commandes + config)

```cpp
// src/Modules/ExampleModule/ExampleModule.h
#pragma once
#include "Core/Module.h"
#include "Core/Services/Services.h"

struct ExampleConfig {
    bool enabled = true;
    int32_t sampleRateMs = 1000;
    char apiKey[32] = "";
};

class ExampleModule : public Module {
public:
    const char* moduleId() const override { return "example"; }
    const char* taskName() const override { return "example"; }

    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "datastore";
        return nullptr;
    }

    void init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) override;
    void loop() override;

private:
    ExampleConfig cfgData;

    const LogHubService* logHub = nullptr;
    const CommandService* cmdSvc = nullptr;
    const DataStoreService* dsSvc = nullptr;

    ConfigVariable<bool,0> enabledVar {
        NVS_KEY("ex_en"),"enabled","example",ConfigType::Bool,
        &cfgData.enabled,ConfigPersistence::Persistent,0
    };
    ConfigVariable<int32_t,0> rateVar {
        NVS_KEY("ex_rate"),"sampleRateMs","example",ConfigType::Int32,
        &cfgData.sampleRateMs,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> apiKeyVar {
        NVS_KEY("ex_key"),"apiKey","example",ConfigType::CharArray,
        cfgData.apiKey,ConfigPersistence::Persistent,sizeof(cfgData.apiKey)
    };

    static bool cmdPing(void* userCtx, const CommandRequest& req, char* reply, size_t replyLen);
};
```

```cpp
// src/Modules/ExampleModule/ExampleModule.cpp
#include "ExampleModule.h"
#define LOG_TAG "ExampMod"
#include "Core/ModuleLog.h"

void ExampleModule::init(ConfigStore& cfg, I2CManager&, ServiceRegistry& services) {
    cfg.registerVar(enabledVar);
    cfg.registerVar(rateVar);
    cfg.registerVar(apiKeyVar);

    logHub = services.get<LogHubService>("loghub");
    cmdSvc = services.get<CommandService>("cmd");
    dsSvc  = services.get<DataStoreService>("datastore");

    if (cmdSvc) {
        cmdSvc->registerHandler(cmdSvc->ctx, "example.ping", cmdPing, this);
    }

    LOGI("ExampleModule init");
}

void ExampleModule::loop() {
    if (!cfgData.enabled) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
    }

    // Exemple de log périodique
    LOGD("tick rate=%ld", (long)cfgData.sampleRateMs);
    vTaskDelay(pdMS_TO_TICKS(cfgData.sampleRateMs));
}

bool ExampleModule::cmdPing(void*, const CommandRequest&, char* reply, size_t replyLen) {
    snprintf(reply, replyLen, "{\"ok\":true,\"pong\":true}");
    return true;
}
```

### 1.3 Enregistrement dans `main.cpp`

```cpp
static ExampleModule exampleModule;
// ...
moduleManager.add(&exampleModule);
```

---

## 2) Ajouter des variables runtime dans le DataStore

### 2.1 Étendre le modèle runtime

Modifie `src/Core/DataModel.h` :

```cpp
struct RuntimeNetwork {
    bool ready = false;
    IpV4 ip {0,0,0,0};
    int8_t rssi = 0;      // <-- nouveau champ
};

struct RuntimeData {
    RuntimeNetwork network;
    // Ajoute ici d’autres blocs runtime (sensors, controls, etc.)
};
```

### 2.2 Déclarer un DataId et DirtyFlag

Modifie `src/Core/EventBus/EventPayloads.h` :

```cpp
enum class DataId : uint16_t {
    NetworkReady = 1,
    NetworkIp = 2,
    NetworkRssi = 3   // <-- nouveau
};

enum DirtyFlags : uint32_t {
    DIRTY_NONE    = 0,
    DIRTY_NETWORK = 1 << 0,
};
```

### 2.3 Ajouter getters/setters dans DataStore

Modifie `src/Core/DataStore/DataStore.h` :

```cpp
int8_t networkRssi() const { return _rt.network.rssi; }
void setNetworkRssi(int8_t rssi);
```

Modifie `src/Core/DataStore/DataStore.cpp` :

```cpp
void DataStore::setNetworkRssi(int8_t rssi)
{
    if (_rt.network.rssi == rssi) return;
    _rt.network.rssi = rssi;

    markDirty(DIRTY_NETWORK);
    publishChanged(DataId::NetworkRssi);
    publishSnapshot();
}
```

---

## 3) Poster un événement dans l’EventBus

### 3.1 Définir un payload dédié

Modifie `src/Core/EventBus/EventPayloads.h` :

```cpp
struct PumpDosePayload {
    uint8_t pumpId;
    uint32_t durationMs;
};
```

Ajoute un ID d’évènement dans `src/Core/EventBus/EventId.h` :

```cpp
enum class EventId : uint16_t {
    // ...
    DoseStarted = 500,
};
```

### 3.2 Poster l’évènement

Dans un module :

```cpp
PumpDosePayload p{.pumpId = 1, .durationMs = 30000};
if (eventBus) {
    eventBus->post(EventId::DoseStarted, &p, sizeof(p));
}
```

### 3.3 S’abonner à l’évènement

```cpp
void ExampleModule::init(ConfigStore& cfg, I2CManager&, ServiceRegistry& services) {
    // ...
    auto* ebSvc = services.get<EventBusService>("eventbus");
    if (ebSvc && ebSvc->bus) {
        ebSvc->bus->subscribe(EventId::DoseStarted, &ExampleModule::onEventStatic, this);
    }
}

static void onEventStatic(const Event& e, void* user) {
    static_cast<ExampleModule*>(user)->onEvent(e);
}

void ExampleModule::onEvent(const Event& e) {
    if (e.id != EventId::DoseStarted) return;
    const PumpDosePayload* p = (const PumpDosePayload*)e.payload;
    if (!p) return;
    LOGI("DoseStarted pump=%u durationMs=%lu", p->pumpId, (unsigned long)p->durationMs);
}
```


