# Flow.IO

Flow.IO is a modular ESP32-based automation platform built to run a complete pool maintenance system: sensing, actuation, scheduling, and water-quality control—packaged as a scalable set of runtime modules.

It provides a foundation for:
- Equipment automation (pumps, filtration, valves, robot, filling, relays)
- Water quality monitoring (temperature, pressure, ORP, pH, level)
- Scheduling (daily/weekly slots, edge events, replay at boot)
- Remote supervision and integration via MQTT
- Home Assistant discovery and entities
- Safe, deterministic embedded execution with bounded resources

## What Flow.IO Delivers

- **Modular architecture**: add or replace behaviors as modules without rewriting the core.
- **Deterministic runtime**: periodic control loops and bounded queues/buffers.
- **Strong separation of concerns**: hardware IO vs. business logic vs. networking.
- **Operational visibility**: structured logs, error codes, runtime snapshots.

## Quick Start (PlatformIO)

1. Open the project in VS Code with PlatformIO.
2. Configure board/environment in `platformio.ini`.
3. Build and upload.
4. Configure Wi‑Fi / MQTT using the config interface (MQTT `cfg/set`) or through your preferred provisioning flow.

## Documentation Index

- [Code Architecture](./CODE_ARCHITECTURE.md)
- [Core Services](./core/CoreServices.md)
- [Core Data Model: DataStore & DataKeys](./core/DataStore.md)
- [Configuration & Persistence: ConfigStore & NVS Keys](./core/ConfigStore.md)
- [Events: EventBus](./core/EventBus.md)
- [Logging](./core/Logging.md)
- [Error Codes](./core/ErrorCodes.md)
- [SystemLimits](./core/SystemLimits.md)
- [Module Development Guide](./core/ModuleDevelopmentGuide.md)

### Module Pages
- [SystemModule](./modules/SystemModule.md)
- [SystemMonitorModule](./modules/SystemMonitorModule.md)
- [LogHubModule](./modules/LogHubModule.md)
- [LogDispatcherModule](./modules/LogDispatcherModule.md)
- [LogSerialSinkModule](./modules/LogSerialSinkModule.md)
- [EventBusModule](./modules/EventBusModule.md)
- [DataStoreModule](./modules/DataStoreModule.md)
- [ConfigStoreModule](./modules/ConfigStoreModule.md)
- [CommandModule](./modules/CommandModule.md)
- [WifiModule](./modules/WifiModule.md)
- [TimeModule](./modules/TimeModule.md)
- [MQTTModule](./modules/MQTTModule.md)
- [HAModule](./modules/HAModule.md)
- [IOModule](./modules/IOModule.md)
- [PoolDeviceModule](./modules/PoolDeviceModule.md)
- [PoolLogicModule](./modules/PoolLogicModule.md)

### Specifications
- [MQTT Protocol](./specs/MQTT_PROTOCOL.md)
- [Time Scheduler Model](./specs/TIME_SCHEDULER.md)
- [IO Model](./specs/IO_MODEL.md)

---

## Quality Gate Summary (per module)

| Module | Type | G1 | G2 | G3 | G4 | G5 | G6 | G7 | G8 | G9 | G10 | Overall |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| SystemModule | Active | 9.0 | 9.0 | 8.5 | 9.0 | 8.5 | 8.5 | 9.0 | 8.0 | 7.5 | 8.5 | 8.70 |
| SystemMonitorModule | Active | 9.0 | 9.0 | 8.5 | 9.0 | 8.5 | 9.0 | 8.5 | 8.0 | 7.0 | 8.5 | 8.68 |
| LogHubModule | Active | 8.5 | 9.0 | 8.0 | 9.0 | 9.0 | 9.0 | 8.5 | 8.0 | 7.5 | 8.5 | 8.60 |
| LogDispatcherModule | Active | 9.0 | 9.0 | 8.0 | 9.0 | 9.0 | 8.5 | 8.5 | 8.0 | 7.0 | 8.5 | 8.63 |
| LogSerialSinkModule | Active | 9.0 | 9.0 | 8.0 | 9.0 | 9.0 | 8.0 | 8.0 | 8.0 | 7.0 | 8.0 | 8.50 |
| EventBusModule | Active | 8.5 | 9.0 | 8.5 | 9.0 | 9.0 | 8.5 | 8.5 | 8.0 | 7.5 | 8.5 | 8.60 |
| DataStoreModule | Passive | 9.0 | 9.0 | 9.0 | 9.0 | 9.0 | 8.5 | 8.5 | 8.5 | 7.5 | 8.5 | 8.78 |
| ConfigStoreModule | Passive | 9.0 | 9.0 | 9.0 | 8.5 | 9.0 | 9.0 | 8.5 | 9.5 | 7.5 | 9.0 | 8.85 |
| CommandModule | Passive | 9.0 | 9.0 | 8.5 | 8.5 | 9.0 | 9.0 | 8.5 | 8.0 | 7.5 | 8.5 | 8.70 |
| WifiModule | Active | 8.5 | 9.0 | 8.5 | 9.0 | 8.5 | 8.5 | 8.0 | 9.0 | 7.0 | 8.5 | 8.53 |
| TimeModule | Active | 9.0 | 9.0 | 8.5 | 8.5 | 8.5 | 9.0 | 8.5 | 9.0 | 7.5 | 8.5 | 8.70 |
| MQTTModule | Active | 8.5 | 9.0 | 8.5 | 9.0 | 9.0 | 9.0 | 8.5 | 9.0 | 7.5 | 9.0 | 8.73 |
| HAModule | Passive | 9.0 | 9.0 | 8.5 | 9.0 | 8.5 | 8.5 | 8.5 | 8.5 | 7.0 | 8.5 | 8.65 |
| IOModule | Active | 9.0 | 9.0 | 9.5 | 9.0 | 9.0 | 9.0 | 9.0 | 9.0 | 8.0 | 8.5 | 8.98 |
| PoolDeviceModule | Passive | 9.0 | 9.0 | 8.5 | 8.5 | 9.0 | 9.0 | 8.5 | 9.0 | 7.5 | 8.5 | 8.75 |
| PoolLogicModule | Active | 9.0 | 9.0 | 8.5 | 9.0 | 9.0 | 9.0 | 9.0 | 9.0 | 8.5 | 8.5 | 8.90 |
