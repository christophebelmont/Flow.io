
# Flow.IO – API Reference Documentation

This documentation provides a complete API-style reference of all modules,
including DataKeys, EventBus usage, configuration keys, services, and business logic.

---

## Quality Gate Summary

| Module | Overall |
|--------|---------|
| PoolLogicModule | 8.9 |
| IOModule | 9.0 |
| MQTTModule | 8.8 |
| TimeModule | 8.6 |
| ConfigStoreModule | 9.0 |
| DataStoreModule | 8.7 |
| EventBusModule | 8.6 |
| WifiModule | 8.4 |
| LogHub / Dispatcher / Sinks | 8.4 |
| PoolDeviceModule | 8.6 |
| HAModule | 8.5 |
| CommandModule | 8.6 |
| System / Monitor | 8.6 |

---

## Quality Gate Definitions

**G1 – Memory Discipline**  
No dynamic heap allocation in steady state loops. All buffers and JSON documents are statically bounded.

**G2 – Service Coupling Only**  
Modules communicate exclusively through declared services and EventBus, never through direct cross-module calls.

**G3 – Static Topology**  
All runtime structures (slots, publishers, endpoints, queues) have compile-time bounded sizes.

**G4 – Bounded Serialization**  
All JSON parsing/serialization uses StaticJsonDocument with explicit size limits defined in SystemLimits.

**G5 – No Magic Numbers**  
Constants are centralized in SystemLimits, Layout, Domain, or NvsKeys headers.

**G6 – Observability**  
All modules expose runtime state via DataStore and standardized error payloads.

**G7 – Deterministic Timing**  
Loop periods, backoff, and scheduler ticks are bounded and explicitly defined.

**G8 – Configuration Hygiene**  
All persistent configuration keys are centralized in NvsKeys and registered via ConfigVariable.

**G9 – Testability**  
Business logic is separable into deterministic functions and unit-testable components.

**G10 – API & Documentation Consistency**  
Public services, DataKeys, NvsKeys, and topics are centrally declared and documented.


---

## Documentation Index

- [Core Services](./core/CoreServices.md)
- [Business Logic – PoolLogic](./specs/POOL_LOGIC_DETAILED.md)
- Module Reference (see `modules/` folder)
