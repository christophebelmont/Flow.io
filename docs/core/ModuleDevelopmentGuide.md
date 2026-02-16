# Module Development Guide

This guide explains how to implement a new Flow.IO module that integrates cleanly with Core services.

## 1. Choose Module Type

### Active module

Use `Module` when you need a loop or background task.

```cpp
class MyActiveModule : public Module {
public:
  const char* moduleId() const override { return "my.module"; }
  const char* taskName() const override { return "my.module"; }
  void init(ConfigStore& cfg, ServiceRegistry& services) override;
  void loop() override;
};
```

### Passive module

Use `ModulePassive` when behavior is event/service driven.

```cpp
class MyPassiveModule : public ModulePassive {
public:
  const char* moduleId() const override { return "my.passive"; }
  void init(ConfigStore& cfg, ServiceRegistry& services) override;
};
```

## 2. Declare Dependencies

Declare dependencies via `dependencyCount()/dependency()` so the ModuleManager can validate wiring:

```cpp
uint8_t dependencyCount() const override { return 2; }
const char* dependency(uint8_t i) const override {
  if (i == 0) return "loghub";
  if (i == 1) return "datastore";
  return nullptr;
}
```

## 3. Acquire Services

Use `services.get<T>("id")` in `init()`:

```cpp
logHub = services.get<LogHubService>("loghub");
dataStore = services.get<DataStoreService>("datastore")->store;
```

## 4. ConfigStore variables

Register typed config variables using `NvsKeys` constants and `ConfigVariable<T>`.

## 5. DataStore publishing

Write to `RuntimeData` then notify:

```cpp
auto& rt = dataStore->dataMutable();
rt.my.value = 42;
dataStore->notifyChanged(DataKeys::MyValue, DIRTY_SENSORS);
```

## 6. EventBus subscriptions

Subscribe to relevant events and keep handler work bounded.

## 7. Bounded buffers & JSON

- Use `SystemLimits.h` for sizes
- Use `StaticJsonDocument` with compile-time capacities
- Avoid heap allocations in loops

## 8. Add documentation

Add a module page under `docs/modules/` and document:
- service ids consumed/exposed
- config keys
- DataKeys
- events subscribed/published
- threading behavior
