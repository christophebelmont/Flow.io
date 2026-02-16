# Code Architecture

## High-Level Layers

Flow.IO is structured as:

1. **Core** (`src/Core`, `include/Core`)
   - Service registry & typed service interfaces
   - EventBus (internal events)
   - DataStore (runtime state + snapshots)
   - ConfigStore (persistent configuration, NVS-backed)
   - Logging framework (LogHub + sinks)
   - Common types, limits, error codes

2. **Modules** (`src/Modules`)
   - Active modules: own FreeRTOS task (`Module`)
   - Passive modules: no task, react through services/events (`ModulePassive`)

3. **Board & Domain Configuration** (`include/Board`, `include/Domain`, `include/Core/Layout`)
   - Pin mapping and board layout constants
   - Pool equipment mapping (logical slots → IO ids)
   - Domain defaults (thresholds, calibration, etc.)

4. **Application Wiring** (`src/main.cpp`)
   - Initializes NVS namespace
   - Instantiates modules
   - Registers dependencies/services
   - Connects runtime snapshot publishers to MQTT

## Runtime Model

### Active modules

Active modules inherit `Module` and implement:

```cpp
void init(ConfigStore& cfg, ServiceRegistry& services) override;
void loop() override;
```

A FreeRTOS task is created via `Module::startTask()`.

### Passive modules

Passive modules inherit `ModulePassive` and implement only initialization:

```cpp
void init(ConfigStore& cfg, ServiceRegistry& services) override;
bool hasTask() const override { return false; }
```

### Data & Events Flow

- Physical signals → **IOModule** → **DataStore**
- Business decisions → **PoolLogicModule** → **PoolDeviceModule** / **IOModule**
- Notifications → **EventBus** → **MQTTModule**
- Persistent configuration → **ConfigStore** → Modules (via `ConfigChanged` events)

## Service Registry

Modules expose and consume capabilities using typed service structs:

```cpp
services.add("mqtt", &mqttService);
auto mqtt = services.get<MqttService>("mqtt");
```

The recommended pattern is:
- declare dependencies via `dependencyCount()/dependency()`
- retrieve services by id inside `init()`
- never call another module implementation directly
