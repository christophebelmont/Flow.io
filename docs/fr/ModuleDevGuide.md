# ModuleDevGuide

Practical guide to create a new module in Flow.IO with real patterns already used in the repository.

## 1) Pick the right module type

Use `Module` when you need runtime behavior in a task loop.

Real example (`WifiModule`):

```cpp
class WifiModule : public Module {
public:
    const char* moduleId() const override { return "wifi"; }
    const char* taskName() const override { return "wifi"; }
    void init(ConfigStore& cfg, ServiceRegistry& services) override;
    void loop() override;
};
```

Use `ModulePassive` when the module only wires services/commands/config and does not need its own task.

Real example: `CommandModule`, `ConfigStoreModule`, `DataStoreModule`, `SystemModule`.

## 2) Define module identity and dependencies

Every module must expose:

- a stable `moduleId()`
- a `taskName()` (for active modules)
- explicit dependencies through `dependencyCount()` and `dependency(i)`

Real example (`TimeModule`):

```cpp
uint8_t dependencyCount() const override { return 4; }
const char* dependency(uint8_t i) const override {
    if (i == 0) return "loghub";
    if (i == 1) return "datastore";
    if (i == 2) return "cmd";
    if (i == 3) return "eventbus";
    return nullptr;
}
```

## 3) Register ConfigStore variables

Define `ConfigVariable` members in the module and register them in `init()`.

Real example (`MQTTModule`):

```cpp
ConfigVariable<char,0> hostVar {
    NVS_KEY("mq_host"),"host","mqtt",ConfigType::CharArray,
    (char*)cfgData.host,ConfigPersistence::Persistent,sizeof(cfgData.host)
};

void MQTTModule::init(ConfigStore& cfg, ServiceRegistry& services) {
    cfg.registerVar(hostVar);
    cfg.registerVar(portVar);
    cfg.registerVar(userVar);
    cfg.registerVar(passVar);
    cfg.registerVar(baseTopicVar);
    cfg.registerVar(enabledVar);
    cfg.registerVar(sensorMinVar);
    ...
}
```

Guidelines:

- keep `moduleName` consistent (`mqtt`, `time/scheduler`, `io/input/a0`, `pdm/pd0`)
- never write NVS directly from module logic

## 4) Provide and consume services

### 4.1 Provide a service

Real example (`MQTTModule` provides `mqtt`):

```cpp
mqttSvc.publish = MQTTModule::svcPublish;
mqttSvc.formatTopic = MQTTModule::svcFormatTopic;
mqttSvc.isConnected = MQTTModule::svcIsConnected;
mqttSvc.ctx = this;
services.add("mqtt", &mqttSvc);
```

### 4.2 Consume services

Real example (`HAModule`):

```cpp
eventBusSvc = services.get<EventBusService>("eventbus");
cfgSvc = services.get<ConfigStoreService>("config");
dsSvc = services.get<DataStoreService>("datastore");
mqttSvc = services.get<MqttService>("mqtt");
```

Always handle `nullptr` before use.

## 5) Use EventBus for async orchestration

Subscribe in `init()`, keep callbacks lightweight, defer heavy work to `loop()`.

Real example (`PoolDeviceModule`):

```cpp
if (eventBus_) {
    eventBus_->subscribe(EventId::SchedulerEventTriggered, &PoolDeviceModule::onEventStatic_, this);
}
```

The callback only updates a pending mask; resets are applied later in `tickDevices_()`.

## 6) Publish runtime state in DataStore

When a module owns shared runtime state, add:

1. `<Name>ModuleDataModel.h` with `MODULE_DATA_MODEL`
2. `<Name>Runtime.h` with typed helpers and DataKeys
3. notifications through `ds.notifyChanged(...)`

Real helper example (`WifiRuntime.h`):

```cpp
constexpr DataKey DATAKEY_WIFI_READY = 1;

static inline void setWifiReady(DataStore& ds, bool ready)
{
    RuntimeData& rt = ds.dataMutable();
    if (rt.wifi.ready == ready) return;
    rt.wifi.ready = ready;
    ds.notifyChanged(DATAKEY_WIFI_READY, DIRTY_NETWORK);
}
```

Real usage example (`WifiModule.cpp`):

```cpp
if (dataStore) {
    setWifiIp(*dataStore, ip4);
    setWifiReady(*dataStore, true);
}
```

After adding new runtime model files, regenerate aggregated headers:

```bash
python3 scripts/generate_datamodel.py
```

## 7) Expose runtime snapshots when needed

If your module publishes structured runtime payloads through MQTT routing, implement `IRuntimeSnapshotProvider`.

Real examples:

- `IOModule` exposes snapshots like `rt/io/input/aN` and `rt/io/output/dN`
- `PoolDeviceModule` exposes snapshots like `rt/pdm/pdN`

## 8) Register commands (if needed)

Use `CommandService` for external control surface.

Real examples:

- `SystemModule`: `system.ping`, `system.reboot`, `system.factory_reset`
- `IOModule`: `io.write`
- `PoolDeviceModule`: `pool.write`, `pool.refill`
- `TimeModule`: `time.resync`, `time.scheduler.*`

Pattern:

```cpp
cmdSvc->registerHandler(cmdSvc->ctx, "time.scheduler.set", cmdSchedSet, this);
```

## 9) Wire the module in `main.cpp`

Add instance and register it in `ModuleManager`.

Real example:

```cpp
moduleManager.add(&wifiModule);
moduleManager.add(&timeModule);
moduleManager.add(&mqttModule);
moduleManager.add(&ioModule);
moduleManager.add(&poolDeviceModule);
```

For domain modules, wire concrete definitions in `main.cpp` (real project pattern):

- `ioModule.defineAnalogInput(...)`
- `ioModule.defineDigitalOutput(...)`
- `poolDeviceModule.defineDevice(...)`

## 10) Mandatory module documentation

For each new module, add `docs/en/modules/<Name>Module.md` and `docs/fr/modules/<Name>Module.md` with the same structure:

1. `General description`
2. `Module dependencies`
3. `Provided services`
4. `Consumed services`
5. `ConfigStore values used`
6. `DataStore values used`

## 11) Merge checklist

- module compiles (`pio run`)
- dependencies are accurate
- service registration/consumption is validated
- ConfigStore keys are documented
- DataStore keys/fields are documented
- module docs are added in EN and FR indexes
- README links are updated
