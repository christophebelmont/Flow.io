# Core Services

This page describes the foundational services used by modules.

## Service Registry

`ServiceRegistry` acts as the dependency injection container.

Typical usage:

```cpp
void MyModule::init(ConfigStore& cfg, ServiceRegistry& services) {
  auto log = services.get<LogHubService>("loghub");
  auto bus = services.get<EventBusService>("eventbus");
}
```

Services are small C-style structs (function pointers + context pointer) defined under `include/Core/Services/` and aggregated in `Core/Services/Services.h`.

## Module base classes

### Active modules (`Module`)

`Module` owns a FreeRTOS task. It repeatedly calls `loop()` and enforces a short delay:

```cpp
while (true) {
  self->loop();
  vTaskDelay(pdMS_TO_TICKS(10));
}
```

### Passive modules (`ModulePassive`)

`ModulePassive` provides initialization only and does not create a task.

## Core building blocks

- **EventBus**: bounded internal event dispatch
- **DataStore**: runtime data model and notifications
- **ConfigStore**: typed configuration registry and NVS persistence
- **LogHub + sinks**: structured logging pipeline
- **ErrorCodes**: common error code enum + JSON encoding helper
- **SystemLimits**: shared compile-time constraints
